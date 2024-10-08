// AsyncJson.h
/*
  Async Response to use with ArduinoJson and AsyncWebServer
  Written by Andrew Melvin (SticilFace) with help from me-no-dev and BBlanchon.

  Example of callback in use

   server.on("/json", HTTP_ANY, [](AsyncWebServerRequest * request) {

    AsyncJsonResponse * response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    root["key1"] = "key number one";
    JsonObject& nested = root.createNestedObject("nested");
    nested["key1"] = "key number one";

    response->setLength();
    request->send(response);
  });

  --------------------

  Async Request to use with ArduinoJson and AsyncWebServer
  Written by Arsène von Wyss (avonwyss)

  Example

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/rest/endpoint");
  handler->onRequest([](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    // ...
  });
  server.addHandler(handler);

*/
#ifndef ASYNC_JSON_H_
#define ASYNC_JSON_H_
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "ChunkPrint.h"

#if ARDUINOJSON_VERSION_MAJOR == 6
  #ifndef DYNAMIC_JSON_DOCUMENT_SIZE
    #define DYNAMIC_JSON_DOCUMENT_SIZE 1024
  #endif
#endif

constexpr const char* JSON_MIMETYPE = "application/json";

/*
 * Json Response
 * */

class AsyncJsonResponse : public AsyncAbstractResponse {
  protected:
#if ARDUINOJSON_VERSION_MAJOR == 5
    DynamicJsonBuffer _jsonBuffer;
#elif ARDUINOJSON_VERSION_MAJOR == 6
    DynamicJsonDocument _jsonBuffer;
#else
    JsonDocument _jsonBuffer;
#endif

    JsonVariant _root;
    bool _isValid;

  public:
#if ARDUINOJSON_VERSION_MAJOR == 5
    AsyncJsonResponse(bool isArray = false) : _isValid{false} {
      _code = 200;
      _contentType = JSON_MIMETYPE;
      if (isArray)
        _root = _jsonBuffer.createArray();
      else
        _root = _jsonBuffer.createObject();
    }
#elif ARDUINOJSON_VERSION_MAJOR == 6
    AsyncJsonResponse(bool isArray = false, size_t maxJsonBufferSize = DYNAMIC_JSON_DOCUMENT_SIZE) : _jsonBuffer(maxJsonBufferSize), _isValid{false} {
      _code = 200;
      _contentType = JSON_MIMETYPE;
      if (isArray)
        _root = _jsonBuffer.createNestedArray();
      else
        _root = _jsonBuffer.createNestedObject();
    }
#else
    AsyncJsonResponse(bool isArray = false) : _isValid{false} {
      _code = 200;
      _contentType = JSON_MIMETYPE;
      if (isArray)
        _root = _jsonBuffer.add<JsonArray>();
      else
        _root = _jsonBuffer.add<JsonObject>();
    }
#endif

    JsonVariant& getRoot() { return _root; }
    bool _sourceValid() const { return _isValid; }
    size_t setLength() {

#if ARDUINOJSON_VERSION_MAJOR == 5
      _contentLength = _root.measureLength();
#else
      _contentLength = measureJson(_root);
#endif

      if (_contentLength) {
        _isValid = true;
      }
      return _contentLength;
    }

    size_t getSize() const { return _jsonBuffer.size(); }

#if ARDUINOJSON_VERSION_MAJOR >= 6
    bool overflowed() const { return _jsonBuffer.overflowed(); }
#endif

    size_t _fillBuffer(uint8_t* data, size_t len) {
      ChunkPrint dest(data, _sentLength, len);

#if ARDUINOJSON_VERSION_MAJOR == 5
      _root.printTo(dest);
#else
      serializeJson(_root, dest);
#endif
      return len;
    }
};

class PrettyAsyncJsonResponse : public AsyncJsonResponse {
  public:
#if ARDUINOJSON_VERSION_MAJOR == 6
    PrettyAsyncJsonResponse(bool isArray = false, size_t maxJsonBufferSize = DYNAMIC_JSON_DOCUMENT_SIZE) : AsyncJsonResponse{isArray, maxJsonBufferSize} {}
#else
    PrettyAsyncJsonResponse(bool isArray = false) : AsyncJsonResponse{isArray} {}
#endif
    size_t setLength() {
#if ARDUINOJSON_VERSION_MAJOR == 5
      _contentLength = _root.measurePrettyLength();
#else
      _contentLength = measureJsonPretty(_root);
#endif
      if (_contentLength) {
        _isValid = true;
      }
      return _contentLength;
    }
    size_t _fillBuffer(uint8_t* data, size_t len) {
      ChunkPrint dest(data, _sentLength, len);
#if ARDUINOJSON_VERSION_MAJOR == 5
      _root.prettyPrintTo(dest);
#else
      serializeJsonPretty(_root, dest);
#endif
      return len;
    }
};

typedef std::function<void(AsyncWebServerRequest* request, JsonVariant& json)> ArJsonRequestHandlerFunction;

class AsyncCallbackJsonWebHandler : public AsyncWebHandler {
  private:
  protected:
    const String _uri;
    WebRequestMethodComposite _method;
    ArJsonRequestHandlerFunction _onRequest;
    size_t _contentLength;
#if ARDUINOJSON_VERSION_MAJOR == 6
    const size_t maxJsonBufferSize;
#endif
    size_t _maxContentLength;

  public:
#if ARDUINOJSON_VERSION_MAJOR == 6
    AsyncCallbackJsonWebHandler(const String& uri, ArJsonRequestHandlerFunction onRequest = nullptr, size_t maxJsonBufferSize = DYNAMIC_JSON_DOCUMENT_SIZE)
        : _uri(uri), _method(HTTP_GET | HTTP_POST | HTTP_PUT | HTTP_PATCH), _onRequest(onRequest), maxJsonBufferSize(maxJsonBufferSize), _maxContentLength(16384) {}
#else
    AsyncCallbackJsonWebHandler(const String& uri, ArJsonRequestHandlerFunction onRequest = nullptr)
        : _uri(uri), _method(HTTP_GET | HTTP_POST | HTTP_PUT | HTTP_PATCH), _onRequest(onRequest), _maxContentLength(16384) {}
#endif

    void setMethod(WebRequestMethodComposite method) { _method = method; }
    void setMaxContentLength(int maxContentLength) { _maxContentLength = maxContentLength; }
    void onRequest(ArJsonRequestHandlerFunction fn) { _onRequest = fn; }

    virtual bool canHandle(AsyncWebServerRequest* request) override final {
      if (!_onRequest)
        return false;

      WebRequestMethodComposite request_method = request->method();
      if (!(_method & request_method))
        return false;

      if (_uri.length() && (_uri != request->url() && !request->url().startsWith(_uri + "/")))
        return false;

      if (request_method != HTTP_GET && !request->contentType().equalsIgnoreCase(JSON_MIMETYPE))
        return false;

      return true;
    }

    virtual void handleRequest(AsyncWebServerRequest* request) override final {
      if (_onRequest) {
        if (request->method() == HTTP_GET) {
          JsonVariant json;
          _onRequest(request, json);
          return;
        } else if (request->_tempObject != NULL) {

#if ARDUINOJSON_VERSION_MAJOR == 5
          DynamicJsonBuffer jsonBuffer;
          JsonVariant json = jsonBuffer.parse((uint8_t*)(request->_tempObject));
          if (json.success()) {
#elif ARDUINOJSON_VERSION_MAJOR == 6
          DynamicJsonDocument jsonBuffer(this->maxJsonBufferSize);
          DeserializationError error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
          if (!error) {
            JsonVariant json = jsonBuffer.as<JsonVariant>();
#else
          JsonDocument jsonBuffer;
          DeserializationError error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
          if (!error) {
            JsonVariant json = jsonBuffer.as<JsonVariant>();
#endif

            _onRequest(request, json);
            return;
          }
        }
        request->send(_contentLength > _maxContentLength ? 413 : 400);
      } else {
        request->send(500);
      }
    }
    virtual void handleUpload(__unused AsyncWebServerRequest* request, __unused const String& filename, __unused size_t index, __unused uint8_t* data, __unused size_t len, __unused bool final) override final {
    }
    virtual void handleBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) override final {
      if (_onRequest) {
        _contentLength = total;
        if (total > 0 && request->_tempObject == NULL && total < _maxContentLength) {
          request->_tempObject = malloc(total);
        }
        if (request->_tempObject != NULL) {
          memcpy((uint8_t*)(request->_tempObject) + index, data, len);
        }
      }
    }
    virtual bool isRequestHandlerTrivial() override final { return _onRequest ? false : true; }
};
#endif
