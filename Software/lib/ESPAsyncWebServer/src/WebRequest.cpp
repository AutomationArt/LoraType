/*
  Asynchronous WebServer library for Espressif MCUs

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "ESPAsyncWebServer.h"
#include "WebAuthentication.h"
#include "WebResponseImpl.h"
#include "literals.h"
#include <cstring>

#define __is_param_char(c) ((c) && ((c) != '{') && ((c) != '[') && ((c) != '&') && ((c) != '='))

using namespace asyncsrv;

enum { PARSE_REQ_START,
       PARSE_REQ_HEADERS,
       PARSE_REQ_BODY,
       PARSE_REQ_END,
       PARSE_REQ_FAIL };

AsyncWebServerRequest::AsyncWebServerRequest(AsyncWebServer* s, AsyncClient* c)
    : _client(c), _server(s), _handler(NULL), _response(NULL), _temp(), _parseState(0), _version(0), _method(HTTP_ANY), _url(), _host(), _contentType(), _boundary(), _authorization(), _reqconntype(RCT_HTTP), _isDigest(false), _isMultipart(false), _isPlainPost(false), _expectingContinue(false), _contentLength(0), _parsedLength(0), _multiParseState(0), _boundaryPosition(0), _itemStartIndex(0), _itemSize(0), _itemName(), _itemFilename(), _itemType(), _itemValue(), _itemBuffer(0), _itemBufferIndex(0), _itemIsFile(false), _tempObject(NULL) {
  c->onError([](void* r, AsyncClient* c, int8_t error) { (void)c; AsyncWebServerRequest *req = (AsyncWebServerRequest*)r; req->_onError(error); }, this);
  c->onAck([](void* r, AsyncClient* c, size_t len, uint32_t time) { (void)c; AsyncWebServerRequest *req = (AsyncWebServerRequest*)r; req->_onAck(len, time); }, this);
  c->onDisconnect([](void* r, AsyncClient* c) { AsyncWebServerRequest *req = (AsyncWebServerRequest*)r; req->_onDisconnect(); delete c; }, this);
  c->onTimeout([](void* r, AsyncClient* c, uint32_t time) { (void)c; AsyncWebServerRequest *req = (AsyncWebServerRequest*)r; req->_onTimeout(time); }, this);
  c->onData([](void* r, AsyncClient* c, void* buf, size_t len) { (void)c; AsyncWebServerRequest *req = (AsyncWebServerRequest*)r; req->_onData(buf, len); }, this);
  c->onPoll([](void* r, AsyncClient* c) { (void)c; AsyncWebServerRequest *req = ( AsyncWebServerRequest*)r; req->_onPoll(); }, this);
}

AsyncWebServerRequest::~AsyncWebServerRequest() {
  _headers.clear();

  _pathParams.clear();

  if (_response != NULL) {
    delete _response;
  }

  if (_tempObject != NULL) {
    free(_tempObject);
  }

  if (_tempFile) {
    _tempFile.close();
  }

  if (_itemBuffer) {
    free(_itemBuffer);
  }
}

void AsyncWebServerRequest::_onData(void* buf, size_t len) {
  size_t i = 0;
  while (true) {

    if (_parseState < PARSE_REQ_BODY) {
      // Find new line in buf
      char* str = (char*)buf;
      for (i = 0; i < len; i++) {
        if (str[i] == '\n') {
          break;
        }
      }
      if (i == len) { // No new line, just add the buffer in _temp
        char ch = str[len - 1];
        str[len - 1] = 0;
        _temp.reserve(_temp.length() + len);
        _temp.concat(str);
        _temp.concat(ch);
      } else {      // Found new line - extract it and parse
        str[i] = 0; // Terminate the string at the end of the line.
        _temp.concat(str);
        _temp.trim();
        _parseLine();
        if (++i < len) {
          // Still have more buffer to process
          buf = str + i;
          len -= i;
          continue;
        }
      }
    } else if (_parseState == PARSE_REQ_BODY) {
      // A handler should be already attached at this point in _parseLine function.
      // If handler does nothing (_onRequest is NULL), we don't need to really parse the body.
      const bool needParse = _handler && !_handler->isRequestHandlerTrivial();
      if (_isMultipart) {
        if (needParse) {
          size_t i;
          for (i = 0; i < len; i++) {
            _parseMultipartPostByte(((uint8_t*)buf)[i], i == len - 1);
            _parsedLength++;
          }
        } else
          _parsedLength += len;
      } else {
        if (_parsedLength == 0) {
          if (_contentType.startsWith(T_app_xform_urlencoded)) {
            _isPlainPost = true;
          } else if (_contentType == T_text_plain && __is_param_char(((char*)buf)[0])) {
            size_t i = 0;
            while (i < len && __is_param_char(((char*)buf)[i++]))
              ;
            if (i < len && ((char*)buf)[i - 1] == '=') {
              _isPlainPost = true;
            }
          }
        }
        if (!_isPlainPost) {
          if (_handler)
            _handler->handleBody(this, (uint8_t*)buf, len, _parsedLength, _contentLength);
          _parsedLength += len;
        } else if (needParse) {
          size_t i;
          for (i = 0; i < len; i++) {
            _parsedLength++;
            _parsePlainPostChar(((uint8_t*)buf)[i]);
          }
        } else {
          _parsedLength += len;
        }
      }
      if (_parsedLength == _contentLength) {
        _parseState = PARSE_REQ_END;
        _server->_runChain(this, [this]() { return _handler ? _handler->_runChain(this, [this]() { _handler->handleRequest(this); }) : send(501); });
        if (!_sent) {
          if (!_response)
            send(501, T_text_plain, "Handler did not handle the request");
          _client->setRxTimeout(0);
          _response->_respond(this);
          _sent = true;
        }
      }
    }
    break;
  }
}

void AsyncWebServerRequest::_onPoll() {
  // os_printf("p\n");
  if (_response != NULL && _client != NULL && _client->canSend()) {
    if (!_response->_finished()) {
      _response->_ack(this, 0, 0);
    } else {
      AsyncWebServerResponse* r = _response;
      _response = NULL;
      delete r;

      _client->close();
    }
  }
}

void AsyncWebServerRequest::_onAck(size_t len, uint32_t time) {
  // os_printf("a:%u:%u\n", len, time);
  if (_response != NULL) {
    if (!_response->_finished()) {
      _response->_ack(this, len, time);
    } else if (_response->_finished()) {
      AsyncWebServerResponse* r = _response;
      _response = NULL;
      delete r;

      _client->close();
    }
  }
}

void AsyncWebServerRequest::_onError(int8_t error) {
  (void)error;
}

void AsyncWebServerRequest::_onTimeout(uint32_t time) {
  (void)time;
  // os_printf("TIMEOUT: %u, state: %s\n", time, _client->stateToString());
  _client->close();
}

void AsyncWebServerRequest::onDisconnect(ArDisconnectHandler fn) {
  _onDisconnectfn = fn;
}

void AsyncWebServerRequest::_onDisconnect() {
  // os_printf("d\n");
  if (_onDisconnectfn) {
    _onDisconnectfn();
  }
  _server->_handleDisconnect(this);
}

void AsyncWebServerRequest::_addPathParam(const char* p) {
  _pathParams.emplace_back(p);
}

void AsyncWebServerRequest::_addGetParams(const String& params) {
  size_t start = 0;
  while (start < params.length()) {
    int end = params.indexOf('&', start);
    if (end < 0)
      end = params.length();
    int equal = params.indexOf('=', start);
    if (equal < 0 || equal > end)
      equal = end;
    String name(params.substring(start, equal));
    String value(equal + 1 < end ? params.substring(equal + 1, end) : String());
    _params.emplace_back(urlDecode(name), urlDecode(value));
    start = end + 1;
  }
}

bool AsyncWebServerRequest::_parseReqHead() {
  // Split the head into method, url and version
  int index = _temp.indexOf(' ');
  String m = _temp.substring(0, index);
  index = _temp.indexOf(' ', index + 1);
  String u = _temp.substring(m.length() + 1, index);
  _temp = _temp.substring(index + 1);

  if (m == T_GET) {
    _method = HTTP_GET;
  } else if (m == T_POST) {
    _method = HTTP_POST;
  } else if (m == T_DELETE) {
    _method = HTTP_DELETE;
  } else if (m == T_PUT) {
    _method = HTTP_PUT;
  } else if (m == T_PATCH) {
    _method = HTTP_PATCH;
  } else if (m == T_HEAD) {
    _method = HTTP_HEAD;
  } else if (m == T_OPTIONS) {
    _method = HTTP_OPTIONS;
  }

  String g;
  index = u.indexOf('?');
  if (index > 0) {
    g = u.substring(index + 1);
    u = u.substring(0, index);
  }
  _url = urlDecode(u);
  _addGetParams(g);

  if (!_temp.startsWith(T_HTTP_1_0))
    _version = 1;

  _temp = emptyString;
  return true;
}

bool AsyncWebServerRequest::_parseReqHeader() {
  int index = _temp.indexOf(':');
  if (index) {
    String name(_temp.substring(0, index));
    String value(_temp.substring(index + 2));
    if (name.equalsIgnoreCase(T_Host)) {
      _host = value;
    } else if (name.equalsIgnoreCase(T_Content_Type)) {
      _contentType = value.substring(0, value.indexOf(';'));
      if (value.startsWith(T_MULTIPART_)) {
        _boundary = value.substring(value.indexOf('=') + 1);
        _boundary.replace(String('"'), String());
        _isMultipart = true;
      }
    } else if (name.equalsIgnoreCase(T_Content_Length)) {
      _contentLength = atoi(value.c_str());
    } else if (name.equalsIgnoreCase(T_EXPECT) && value == T_100_CONTINUE) {
      _expectingContinue = true;
    } else if (name.equalsIgnoreCase(T_AUTH)) {
      if (value.length() > 5 && value.substring(0, 5).equalsIgnoreCase(T_BASIC)) {
        _authorization = value.substring(6);
      } else if (value.length() > 6 && value.substring(0, 6).equalsIgnoreCase(T_DIGEST)) {
        _isDigest = true;
        _authorization = value.substring(7);
      }
    } else {
      if (name.equalsIgnoreCase(T_UPGRADE) && value.equalsIgnoreCase(T_WS)) {
        // WebSocket request can be uniquely identified by header: [Upgrade: websocket]
        _reqconntype = RCT_WS;
      } else if (name.equalsIgnoreCase(T_ACCEPT)) {
        String lowcase(value);
        lowcase.toLowerCase();
#ifndef ESP8266
        const char* substr = std::strstr(lowcase.c_str(), T_text_event_stream);
#else
        const char* substr = std::strstr(lowcase.c_str(), String(T_text_event_stream).c_str());
#endif
        if (substr != NULL) {
          // WebEvent request can be uniquely identified by header:  [Accept: text/event-stream]
          _reqconntype = RCT_EVENT;
        }
      }
    }
    _headers.emplace_back(name, value);
  }
#ifndef TARGET_RP2040
  _temp.clear();
#else
  // Ancient PRI core does not have String::clear() method 8-()
  _temp = emptyString;
#endif
  return true;
}

void AsyncWebServerRequest::_parsePlainPostChar(uint8_t data) {
  if (data && (char)data != '&')
    _temp += (char)data;
  if (!data || (char)data == '&' || _parsedLength == _contentLength) {
    String name(T_BODY);
    String value(_temp);
    if (!(_temp.charAt(0) == '{') && !(_temp.charAt(0) == '[') && _temp.indexOf('=') > 0) {
      name = _temp.substring(0, _temp.indexOf('='));
      value = _temp.substring(_temp.indexOf('=') + 1);
    }
    _params.emplace_back(urlDecode(name), urlDecode(value), true);

#ifndef TARGET_RP2040
    _temp.clear();
#else
    // Ancient PRI core does not have String::clear() method 8-()
    _temp = emptyString;
#endif
  }
}

void AsyncWebServerRequest::_handleUploadByte(uint8_t data, bool last) {
  _itemBuffer[_itemBufferIndex++] = data;

  if (last || _itemBufferIndex == RESPONSE_STREAM_BUFFER_SIZE) {
    // check if authenticated before calling the upload
    if (_handler)
      _handler->handleUpload(this, _itemFilename, _itemSize - _itemBufferIndex, _itemBuffer, _itemBufferIndex, false);
    _itemBufferIndex = 0;
  }
}

enum {
  EXPECT_BOUNDARY,
  PARSE_HEADERS,
  WAIT_FOR_RETURN1,
  EXPECT_FEED1,
  EXPECT_DASH1,
  EXPECT_DASH2,
  BOUNDARY_OR_DATA,
  DASH3_OR_RETURN2,
  EXPECT_FEED2,
  PARSING_FINISHED,
  PARSE_ERROR
};

void AsyncWebServerRequest::_parseMultipartPostByte(uint8_t data, bool last) {
#define itemWriteByte(b)          \
  do {                            \
    _itemSize++;                  \
    if (_itemIsFile)              \
      _handleUploadByte(b, last); \
    else                          \
      _itemValue += (char)(b);    \
  } while (0)

  if (!_parsedLength) {
    _multiParseState = EXPECT_BOUNDARY;
    _temp = emptyString;
    _itemName = emptyString;
    _itemFilename = emptyString;
    _itemType = emptyString;
  }

  if (_multiParseState == WAIT_FOR_RETURN1) {
    if (data != '\r') {
      itemWriteByte(data);
    } else {
      _multiParseState = EXPECT_FEED1;
    }
  } else if (_multiParseState == EXPECT_BOUNDARY) {
    if (_parsedLength < 2 && data != '-') {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 2 < _boundary.length() && _boundary.c_str()[_parsedLength - 2] != data) {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 2 == _boundary.length() && data != '\r') {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 3 == _boundary.length()) {
      if (data != '\n') {
        _multiParseState = PARSE_ERROR;
        return;
      }
      _multiParseState = PARSE_HEADERS;
      _itemIsFile = false;
    }
  } else if (_multiParseState == PARSE_HEADERS) {
    if ((char)data != '\r' && (char)data != '\n')
      _temp += (char)data;
    if ((char)data == '\n') {
      if (_temp.length()) {
        if (_temp.length() > 12 && _temp.substring(0, 12).equalsIgnoreCase(T_Content_Type)) {
          _itemType = _temp.substring(14);
          _itemIsFile = true;
        } else if (_temp.length() > 19 && _temp.substring(0, 19).equalsIgnoreCase(T_Content_Disposition)) {
          _temp = _temp.substring(_temp.indexOf(';') + 2);
          while (_temp.indexOf(';') > 0) {
            String name = _temp.substring(0, _temp.indexOf('='));
            String nameVal = _temp.substring(_temp.indexOf('=') + 2, _temp.indexOf(';') - 1);
            if (name == T_name) {
              _itemName = nameVal;
            } else if (name == T_filename) {
              _itemFilename = nameVal;
              _itemIsFile = true;
            }
            _temp = _temp.substring(_temp.indexOf(';') + 2);
          }
          String name = _temp.substring(0, _temp.indexOf('='));
          String nameVal = _temp.substring(_temp.indexOf('=') + 2, _temp.length() - 1);
          if (name == T_name) {
            _itemName = nameVal;
          } else if (name == T_filename) {
            _itemFilename = nameVal;
            _itemIsFile = true;
          }
        }
        _temp = emptyString;
      } else {
        _multiParseState = WAIT_FOR_RETURN1;
        // value starts from here
        _itemSize = 0;
        _itemStartIndex = _parsedLength;
        _itemValue = emptyString;
        if (_itemIsFile) {
          if (_itemBuffer)
            free(_itemBuffer);
          _itemBuffer = (uint8_t*)malloc(RESPONSE_STREAM_BUFFER_SIZE);
          if (_itemBuffer == NULL) {
            _multiParseState = PARSE_ERROR;
            return;
          }
          _itemBufferIndex = 0;
        }
      }
    }
  } else if (_multiParseState == EXPECT_FEED1) {
    if (data != '\n') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState = EXPECT_DASH1;
    }
  } else if (_multiParseState == EXPECT_DASH1) {
    if (data != '-') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState = EXPECT_DASH2;
    }
  } else if (_multiParseState == EXPECT_DASH2) {
    if (data != '-') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState = BOUNDARY_OR_DATA;
      _boundaryPosition = 0;
    }
  } else if (_multiParseState == BOUNDARY_OR_DATA) {
    if (_boundaryPosition < _boundary.length() && _boundary.c_str()[_boundaryPosition] != data) {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint8_t i;
      for (i = 0; i < _boundaryPosition; i++)
        itemWriteByte(_boundary.c_str()[i]);
      _parseMultipartPostByte(data, last);
    } else if (_boundaryPosition == _boundary.length() - 1) {
      _multiParseState = DASH3_OR_RETURN2;
      if (!_itemIsFile) {
        _params.emplace_back(_itemName, _itemValue, true);
      } else {
        if (_itemSize) {
          if (_handler)
            _handler->handleUpload(this, _itemFilename, _itemSize - _itemBufferIndex, _itemBuffer, _itemBufferIndex, true);
          _itemBufferIndex = 0;
          _params.emplace_back(_itemName, _itemFilename, true, true, _itemSize);
        }
        free(_itemBuffer);
        _itemBuffer = NULL;
      }

    } else {
      _boundaryPosition++;
    }
  } else if (_multiParseState == DASH3_OR_RETURN2) {
    if (data == '-' && (_contentLength - _parsedLength - 4) != 0) {
      // os_printf("ERROR: The parser got to the end of the POST but is expecting %u bytes more!\nDrop an issue so we can have more info on the matter!\n", _contentLength - _parsedLength - 4);
      _contentLength = _parsedLength + 4; // lets close the request gracefully
    }
    if (data == '\r') {
      _multiParseState = EXPECT_FEED2;
    } else if (data == '-' && _contentLength == (_parsedLength + 4)) {
      _multiParseState = PARSING_FINISHED;
    } else {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint8_t i;
      for (i = 0; i < _boundary.length(); i++)
        itemWriteByte(_boundary.c_str()[i]);
      _parseMultipartPostByte(data, last);
    }
  } else if (_multiParseState == EXPECT_FEED2) {
    if (data == '\n') {
      _multiParseState = PARSE_HEADERS;
      _itemIsFile = false;
    } else {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint8_t i;
      for (i = 0; i < _boundary.length(); i++)
        itemWriteByte(_boundary.c_str()[i]);
      itemWriteByte('\r');
      _parseMultipartPostByte(data, last);
    }
  }
}

void AsyncWebServerRequest::_parseLine() {
  if (_parseState == PARSE_REQ_START) {
    if (!_temp.length()) {
      _parseState = PARSE_REQ_FAIL;
      _client->close();
    } else {
      _parseReqHead();
      _parseState = PARSE_REQ_HEADERS;
    }
    return;
  }

  if (_parseState == PARSE_REQ_HEADERS) {
    if (!_temp.length()) {
      // end of headers
      _server->_rewriteRequest(this);
      _server->_attachHandler(this);
      if (_expectingContinue) {
        String response(T_HTTP_100_CONT);
        _client->write(response.c_str(), response.length());
      }
      if (_contentLength) {
        _parseState = PARSE_REQ_BODY;
      } else {
        _parseState = PARSE_REQ_END;
        _server->_runChain(this, [this]() { return _handler ? _handler->_runChain(this, [this]() { _handler->handleRequest(this); }) : send(501); });
        if (!_sent) {
          if (!_response)
            send(501, T_text_plain, "Handler did not handle the request");
          _client->setRxTimeout(0);
          _response->_respond(this);
          _sent = true;
        }
      }
    } else
      _parseReqHeader();
  }
}

size_t AsyncWebServerRequest::headers() const {
  return _headers.size();
}

bool AsyncWebServerRequest::hasHeader(const char* name) const {
  for (const auto& h : _headers) {
    if (h.name().equalsIgnoreCase(name)) {
      return true;
    }
  }
  return false;
}

#ifdef ESP8266
bool AsyncWebServerRequest::hasHeader(const __FlashStringHelper* data) const {
  return hasHeader(String(data));
}
#endif

const AsyncWebHeader* AsyncWebServerRequest::getHeader(const char* name) const {
  auto iter = std::find_if(std::begin(_headers), std::end(_headers), [&name](const AsyncWebHeader& header) { return header.name().equalsIgnoreCase(name); });
  return (iter == std::end(_headers)) ? nullptr : &(*iter);
}

#ifdef ESP8266
const AsyncWebHeader* AsyncWebServerRequest::getHeader(const __FlashStringHelper* data) const {
  PGM_P p = reinterpret_cast<PGM_P>(data);
  size_t n = strlen_P(p);
  char* name = (char*)malloc(n + 1);
  if (name) {
    strcpy_P(name, p);
    const AsyncWebHeader* result = getHeader(String(name));
    free(name);
    return result;
  } else {
    return nullptr;
  }
}
#endif

const AsyncWebHeader* AsyncWebServerRequest::getHeader(size_t num) const {
  if (num >= _headers.size())
    return nullptr;
  return &(*std::next(_headers.cbegin(), num));
}

size_t AsyncWebServerRequest::params() const {
  return _params.size();
}

bool AsyncWebServerRequest::hasParam(const char* name, bool post, bool file) const {
  for (const auto& p : _params) {
    if (p.name().equals(name) && p.isPost() == post && p.isFile() == file) {
      return true;
    }
  }
  return false;
}

const AsyncWebParameter* AsyncWebServerRequest::getParam(const char* name, bool post, bool file) const {
  for (const auto& p : _params) {
    if (p.name() == name && p.isPost() == post && p.isFile() == file) {
      return &p;
    }
  }
  return nullptr;
}

#ifdef ESP8266
const AsyncWebParameter* AsyncWebServerRequest::getParam(const __FlashStringHelper* data, bool post, bool file) const {
  return getParam(String(data), post, file);
}
#endif

const AsyncWebParameter* AsyncWebServerRequest::getParam(size_t num) const {
  if (num >= _params.size())
    return nullptr;
  return &(*std::next(_params.cbegin(), num));
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(int code, const char* contentType, const char* content, AwsTemplateProcessor callback) {
  if (callback)
    return new AsyncProgmemResponse(code, contentType, (const uint8_t*)content, strlen(content), callback);
  return new AsyncBasicResponse(code, contentType, content);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(int code, const char* contentType, const uint8_t* content, size_t len, AwsTemplateProcessor callback) {
  return new AsyncProgmemResponse(code, contentType, content, len, callback);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(FS& fs, const String& path, const char* contentType, bool download, AwsTemplateProcessor callback) {
  if (fs.exists(path) || (!download && fs.exists(path + T__gz)))
    return new AsyncFileResponse(fs, path, contentType, download, callback);
  return NULL;
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(File content, const String& path, const char* contentType, bool download, AwsTemplateProcessor callback) {
  if (content == true)
    return new AsyncFileResponse(content, path, contentType, download, callback);
  return NULL;
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(Stream& stream, const char* contentType, size_t len, AwsTemplateProcessor callback) {
  return new AsyncStreamResponse(stream, contentType, len, callback);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(const char* contentType, size_t len, AwsResponseFiller callback, AwsTemplateProcessor templateCallback) {
  return new AsyncCallbackResponse(contentType, len, callback, templateCallback);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginChunkedResponse(const char* contentType, AwsResponseFiller callback, AwsTemplateProcessor templateCallback) {
  if (_version)
    return new AsyncChunkedResponse(contentType, callback, templateCallback);
  return new AsyncCallbackResponse(contentType, 0, callback, templateCallback);
}

AsyncResponseStream* AsyncWebServerRequest::beginResponseStream(const char* contentType, size_t bufferSize) {
  return new AsyncResponseStream(contentType, bufferSize);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse_P(int code, const String& contentType, PGM_P content, AwsTemplateProcessor callback) {
  return new AsyncProgmemResponse(code, contentType, (const uint8_t*)content, strlen_P(content), callback);
}

void AsyncWebServerRequest::send(AsyncWebServerResponse* response) {
  if (_sent)
    return;
  if (_response)
    delete _response;
  _response = response;
  if (_response == NULL) {
    _client->close(true);
    _onDisconnect();
    _sent = true;
    return;
  }
  if (!_response->_sourceValid())
    send(500);
}

void AsyncWebServerRequest::redirect(const char* url) {
  AsyncWebServerResponse* response = beginResponse(302);
  response->addHeader(T_LOCATION, url);
  send(response);
}

bool AsyncWebServerRequest::authenticate(const char* username, const char* password, const char* realm, bool passwordIsHash) {
  if (_authorization.length()) {
    if (_isDigest)
      return checkDigestAuthentication(_authorization.c_str(), methodToString(), username, password, realm, passwordIsHash, NULL, NULL, NULL);
    else if (!passwordIsHash)
      return checkBasicAuthentication(_authorization.c_str(), username, password);
    else
      return _authorization.equals(password);
  }
  return false;
}

bool AsyncWebServerRequest::authenticate(const char* hash) {
  if (!_authorization.length() || hash == NULL)
    return false;

  if (_isDigest) {
    String hStr = String(hash);
    int separator = hStr.indexOf(':');
    if (separator <= 0)
      return false;
    String username = hStr.substring(0, separator);
    hStr = hStr.substring(separator + 1);
    separator = hStr.indexOf(':');
    if (separator <= 0)
      return false;
    String realm = hStr.substring(0, separator);
    hStr = hStr.substring(separator + 1);
    return checkDigestAuthentication(_authorization.c_str(), methodToString(), username.c_str(), hStr.c_str(), realm.c_str(), true, NULL, NULL, NULL);
  }

  return (_authorization.equals(hash));
}

void AsyncWebServerRequest::requestAuthentication(const char* realm, bool isDigest) {
  AsyncWebServerResponse* r = beginResponse(401);
  if (!isDigest && realm == NULL) {
    r->addHeader(T_WWW_AUTH, T_BASIC_REALM_LOGIN_REQ);
  } else if (!isDigest) {
    String header(T_BASIC_REALM);
    header.concat(realm);
    header += '"';
    r->addHeader(T_WWW_AUTH, header.c_str());
  } else {
    String header(T_DIGEST_);
    header.concat(requestDigestAuthentication(realm));
    r->addHeader(T_WWW_AUTH, header.c_str());
  }
  send(r);
}

bool AsyncWebServerRequest::hasArg(const char* name) const {
  for (const auto& arg : _params) {
    if (arg.name() == name) {
      return true;
    }
  }
  return false;
}

#ifdef ESP8266
bool AsyncWebServerRequest::hasArg(const __FlashStringHelper* data) const {
  return hasArg(String(data).c_str());
}
#endif

const String& AsyncWebServerRequest::arg(const char* name) const {
  for (const auto& arg : _params) {
    if (arg.name() == name) {
      return arg.value();
    }
  }
  return emptyString;
}

#ifdef ESP8266
const String& AsyncWebServerRequest::arg(const __FlashStringHelper* data) const {
  return arg(String(data).c_str());
}
#endif

const String& AsyncWebServerRequest::arg(size_t i) const {
  return getParam(i)->value();
}

const String& AsyncWebServerRequest::argName(size_t i) const {
  return getParam(i)->name();
}

const String& AsyncWebServerRequest::pathArg(size_t i) const {
  return i < _pathParams.size() ? _pathParams[i] : emptyString;
}

const String& AsyncWebServerRequest::header(const char* name) const {
  const AsyncWebHeader* h = getHeader(name);
  return h ? h->value() : emptyString;
}

#ifdef ESP8266
const String& AsyncWebServerRequest::header(const __FlashStringHelper* data) const {
  return header(String(data).c_str());
};
#endif

const String& AsyncWebServerRequest::header(size_t i) const {
  const AsyncWebHeader* h = getHeader(i);
  return h ? h->value() : emptyString;
}

const String& AsyncWebServerRequest::headerName(size_t i) const {
  const AsyncWebHeader* h = getHeader(i);
  return h ? h->name() : emptyString;
}

String AsyncWebServerRequest::urlDecode(const String& text) const {
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;
  String decoded;
  decoded.reserve(len); // Allocate the string internal buffer - never longer from source text
  while (i < len) {
    char decodedChar;
    char encodedChar = text.charAt(i++);
    if ((encodedChar == '%') && (i + 1 < len)) {
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    } else if (encodedChar == '+') {
      decodedChar = ' ';
    } else {
      decodedChar = encodedChar; // normal ascii char
    }
    decoded.concat(decodedChar);
  }
  return decoded;
}

#ifndef ESP8266
const char* AsyncWebServerRequest::methodToString() const {
  if (_method == HTTP_ANY)
    return T_ANY;
  if (_method & HTTP_GET)
    return T_GET;
  if (_method & HTTP_POST)
    return T_POST;
  if (_method & HTTP_DELETE)
    return T_DELETE;
  if (_method & HTTP_PUT)
    return T_PUT;
  if (_method & HTTP_PATCH)
    return T_PATCH;
  if (_method & HTTP_HEAD)
    return T_HEAD;
  if (_method & HTTP_OPTIONS)
    return T_OPTIONS;
  return T_UNKNOWN;
}
#else  // ESP8266
const __FlashStringHelper* AsyncWebServerRequest::methodToString() const {
  if (_method == HTTP_ANY)
    return FPSTR(T_ANY);
  if (_method & HTTP_GET)
    return FPSTR(T_GET);
  if (_method & HTTP_POST)
    return FPSTR(T_POST);
  if (_method & HTTP_DELETE)
    return FPSTR(T_DELETE);
  if (_method & HTTP_PUT)
    return FPSTR(T_PUT);
  if (_method & HTTP_PATCH)
    return FPSTR(T_PATCH);
  if (_method & HTTP_HEAD)
    return FPSTR(T_HEAD);
  if (_method & HTTP_OPTIONS)
    return FPSTR(T_OPTIONS);
  return FPSTR(T_UNKNOWN);
}
#endif // ESP8266

#ifndef ESP8266
const char* AsyncWebServerRequest::requestedConnTypeToString() const {
  switch (_reqconntype) {
    case RCT_NOT_USED:
      return T_RCT_NOT_USED;
    case RCT_DEFAULT:
      return T_RCT_DEFAULT;
    case RCT_HTTP:
      return T_RCT_HTTP;
    case RCT_WS:
      return T_RCT_WS;
    case RCT_EVENT:
      return T_RCT_EVENT;
    default:
      return T_ERROR;
  }
}
#else  // ESP8266
const __FlashStringHelper* AsyncWebServerRequest::requestedConnTypeToString() const {
  switch (_reqconntype) {
    case RCT_NOT_USED:
      return FPSTR(T_RCT_NOT_USED);
    case RCT_DEFAULT:
      return FPSTR(T_RCT_DEFAULT);
    case RCT_HTTP:
      return FPSTR(T_RCT_HTTP);
    case RCT_WS:
      return FPSTR(T_RCT_WS);
    case RCT_EVENT:
      return FPSTR(T_RCT_EVENT);
    default:
      return FPSTR(T_ERROR);
  }
}
#endif // ESP8266

bool AsyncWebServerRequest::isExpectedRequestedConnType(RequestedConnectionType erct1, RequestedConnectionType erct2, RequestedConnectionType erct3) {
  bool res = false;
  if ((erct1 != RCT_NOT_USED) && (erct1 == _reqconntype))
    res = true;
  if ((erct2 != RCT_NOT_USED) && (erct2 == _reqconntype))
    res = true;
  if ((erct3 != RCT_NOT_USED) && (erct3 == _reqconntype))
    res = true;
  return res;
}
