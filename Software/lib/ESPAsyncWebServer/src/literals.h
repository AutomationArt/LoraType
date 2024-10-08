#pragma once

namespace asyncsrv {

static constexpr const char* empty = "";

#ifndef ESP8622
static constexpr const char* T_100_CONTINUE     = "100-continue";
static constexpr const char* T_ACCEPT           = "Accept";
static constexpr const char* T_Accept_Ranges    = "Accept-Ranges";
static constexpr const char* T_app_xform_urlencoded = "application/x-www-form-urlencoded";
static constexpr const char* T_AUTH             = "Authorization";
static constexpr const char* T_BASIC            = "Basic";
static constexpr const char* T_BASIC_REALM      = "Basic realm=\"";
static constexpr const char* T_BASIC_REALM_LOGIN_REQ = "Basic realm=\"Login Required\"";
static constexpr const char* T_BODY             = "body";
static constexpr const char* T_Cache_Control    = "Cache-Control";
static constexpr const char* T_chunked          = "chunked";
static constexpr const char* T_close            = "close";
static constexpr const char* T_Connection       = "Connection";
static constexpr const char* T_Content_Disposition     = "Content-Disposition";
static constexpr const char* T_Content_Encoding = "Content-Encoding";
static constexpr const char* T_Content_Length   = "Content-Length";
static constexpr const char* T_Content_Type     = "Content-Type";
static constexpr const char* T_Cookie           = "Cookie";
static constexpr const char* T_DIGEST           = "Digest";
static constexpr const char* T_DIGEST_          = "Digest ";
static constexpr const char* T_ETag             = "ETag";
static constexpr const char* T_EXPECT           = "Expect";
static constexpr const char* T_HTTP_1_0         = "HTTP/1.0";
static constexpr const char* T_HTTP_100_CONT    = "HTTP/1.1 100 Continue\r\n\r\n";
static constexpr const char* T_IMS              = "If-Modified-Since";
static constexpr const char* T_INM              = "If-None-Match";
static constexpr const char* T_keep_alive       = "keep-alive";
static constexpr const char* T_Last_Event_ID    = "Last-Event-ID";
static constexpr const char* T_Last_Modified    = "Last-Modified";
static constexpr const char* T_LOCATION         = "Location";
static constexpr const char* T_MULTIPART_       = "multipart/";
static constexpr const char* T_no_cache         = "no-cache";
static constexpr const char* T_none             = "none";
static constexpr const char* T_UPGRADE          = "Upgrade";
static constexpr const char* T_WS               = "websocket";
static constexpr const char* T_WWW_AUTH         = "WWW-Authenticate";
static constexpr const char* T_Transfer_Encoding = "Transfer-Encoding";

// HTTP Methods
static constexpr const char* T_ANY      = "ANY";
static constexpr const char* T_GET      = "GET";
static constexpr const char* T_POST     = "POST";
static constexpr const char* T_PUT      = "PUT";
static constexpr const char* T_DELETE   = "DELETE";
static constexpr const char* T_PATCH    = "PATCH";
static constexpr const char* T_HEAD     = "HEAD";
static constexpr const char* T_OPTIONS  = "OPTIONS";
static constexpr const char* T_UNKNOWN  = "UNKNOWN";

// Req content types
static constexpr const char* T_RCT_NOT_USED = "RCT_NOT_USED";
static constexpr const char* T_RCT_DEFAULT  = "RCT_DEFAULT";
static constexpr const char* T_RCT_HTTP     = "RCT_HTTP";
static constexpr const char* T_RCT_WS       = "RCT_WS";
static constexpr const char* T_RCT_EVENT    = "RCT_EVENT";
static constexpr const char* T_ERROR        = "ERROR";

// extentions & MIME-Types
static constexpr const char* T__css  = ".css";
static constexpr const char* T__eot  = ".eot";
static constexpr const char* T__gif  = ".gif";
static constexpr const char* T__gz   = ".gz";
static constexpr const char* T__htm  = ".htm";
static constexpr const char* T__html = ".html";
static constexpr const char* T__ico  = ".ico";
static constexpr const char* T__jpg  = ".jpg";
static constexpr const char* T__js   = ".js";
static constexpr const char* T__json = ".json";
static constexpr const char* T__pdf  = ".pdf";
static constexpr const char* T__png  = ".png";
static constexpr const char* T__svg  = ".svg";
static constexpr const char* T__ttf  = ".ttf";
static constexpr const char* T__woff = ".woff";
static constexpr const char* T__woff2    = ".woff2";
static constexpr const char* T__xml  = ".xml";
static constexpr const char* T__zip  = ".zip";
static constexpr const char* T_application_javascript   = "application/javascript";
static constexpr const char* T_application_json         = "application/json";
static constexpr const char* T_application_msgpack      = "application/msgpack";
static constexpr const char* T_application_pdf          = "application/pdf";
static constexpr const char* T_application_x_gzip       = "application/x-gzip";
static constexpr const char* T_application_zip          = "application/zip";
static constexpr const char* T_font_eot                 = "font/eot";
static constexpr const char* T_font_ttf                 = "font/ttf";
static constexpr const char* T_font_woff                = "font/woff";
static constexpr const char* T_font_woff2               = "font/woff2";
static constexpr const char* T_image_gif                = "image/gif";
static constexpr const char* T_image_jpeg               = "image/jpeg";
static constexpr const char* T_image_png                = "image/png";
static constexpr const char* T_image_svg_xml            = "image/svg+xml";
static constexpr const char* T_image_x_icon             = "image/x-icon";
static constexpr const char* T_text_css                 = "text/css";
static constexpr const char* T_text_event_stream        = "text/event-stream";
static constexpr const char* T_text_html                = "text/html";
static constexpr const char* T_text_plain               = "text/plain";
static constexpr const char* T_text_xml                 = "text/xml";

// Responce codes
static constexpr const char* T_HTTP_CODE_100 = "Continue";
static constexpr const char* T_HTTP_CODE_101 = "Switching Protocols";
static constexpr const char* T_HTTP_CODE_200 = "OK";
static constexpr const char* T_HTTP_CODE_201 = "Created";
static constexpr const char* T_HTTP_CODE_202 = "Accepted";
static constexpr const char* T_HTTP_CODE_203 = "Non-Authoritative Information";
static constexpr const char* T_HTTP_CODE_204 = "No Content";
static constexpr const char* T_HTTP_CODE_205 = "Reset Content";
static constexpr const char* T_HTTP_CODE_206 = "Partial Content";
static constexpr const char* T_HTTP_CODE_300 = "Multiple Choices";
static constexpr const char* T_HTTP_CODE_301 = "Moved Permanently";
static constexpr const char* T_HTTP_CODE_302 = "Found";
static constexpr const char* T_HTTP_CODE_303 = "See Other";
static constexpr const char* T_HTTP_CODE_304 = "Not Modified";
static constexpr const char* T_HTTP_CODE_305 = "Use Proxy";
static constexpr const char* T_HTTP_CODE_307 = "Temporary Redirect";
static constexpr const char* T_HTTP_CODE_400 = "Bad Request";
static constexpr const char* T_HTTP_CODE_401 = "Unauthorized";
static constexpr const char* T_HTTP_CODE_402 = "Payment Required";
static constexpr const char* T_HTTP_CODE_403 = "Forbidden";
static constexpr const char* T_HTTP_CODE_404 = "Not Found";
static constexpr const char* T_HTTP_CODE_405 = "Method Not Allowed";
static constexpr const char* T_HTTP_CODE_406 = "Not Acceptable";
static constexpr const char* T_HTTP_CODE_407 = "Proxy Authentication Required";
static constexpr const char* T_HTTP_CODE_408 = "Request Time-out";
static constexpr const char* T_HTTP_CODE_409 = "Conflict";
static constexpr const char* T_HTTP_CODE_410 = "Gone";
static constexpr const char* T_HTTP_CODE_411 = "Length Required";
static constexpr const char* T_HTTP_CODE_412 = "Precondition Failed";
static constexpr const char* T_HTTP_CODE_413 = "Request Entity Too Large";
static constexpr const char* T_HTTP_CODE_414 = "Request-URI Too Large";
static constexpr const char* T_HTTP_CODE_415 = "Unsupported Media Type";
static constexpr const char* T_HTTP_CODE_416 = "Requested range not satisfiable";
static constexpr const char* T_HTTP_CODE_417 = "Expectation Failed";
static constexpr const char* T_HTTP_CODE_429 = "Too Many Requests";
static constexpr const char* T_HTTP_CODE_500 = "Internal Server Error";
static constexpr const char* T_HTTP_CODE_501 = "Not Implemented";
static constexpr const char* T_HTTP_CODE_502 = "Bad Gateway";
static constexpr const char* T_HTTP_CODE_503 = "Service Unavailable";
static constexpr const char* T_HTTP_CODE_504 = "Gateway Time-out";
static constexpr const char* T_HTTP_CODE_505 = "HTTP Version not supported";
static constexpr const char* T_HTTP_CODE_ANY = "Unknown code";

// other
static constexpr const char* T__opaque  = "\", opaque=\"";
static constexpr const char* T_13       = "13";
static constexpr const char* T_asyncesp = "asyncesp";
static constexpr const char* T_auth_nonce = "\", qop=\"auth\", nonce=\"";
static constexpr const char* T_cnonce   = "cnonce";
static constexpr const char* T_data_    = "data: ";
static constexpr const char* T_event_   = "event: ";
static constexpr const char* T_filename = "filename";
static constexpr const char* T_gzip     = "gzip";
static constexpr const char* T_Host     = "Host";
static constexpr const char* T_id__     = "id: ";
static constexpr const char* T_name     = "name";
static constexpr const char* T_nc       = "nc";
static constexpr const char* T_nonce    = "nonce";
static constexpr const char* T_opaque   = "opaque";
static constexpr const char* T_qop      = "qop";
static constexpr const char* T_realm    = "realm";
static constexpr const char* T_realm__  = "realm=\"";
static constexpr const char* T_response = "response";
static constexpr const char* T_retry_   = "retry: ";
static constexpr const char* T_rn       = "\r\n";
static constexpr const char* T_rnrn     = "\r\n\r\n";
static constexpr const char* T_uri      = "uri";
static constexpr const char* T_username = "username";


#else   // ESP8622

static const char T_100_CONTINUE[] PROGMEM = "100-continue";
static const char T_ACCEPT[] PROGMEM = "Accept";
static const char T_Accept_Ranges[] PROGMEM = "Accept-Ranges";
static const char T_app_xform_urlencoded[] PROGMEM = "application/x-www-form-urlencoded";
static const char T_AUTH[] PROGMEM = "Authorization";
static const char T_BASIC[] PROGMEM = "Basic";
static const char T_BASIC_REALM[] PROGMEM = "Basic realm=\"";
static const char T_BASIC_REALM_LOGIN_REQ[] PROGMEM = "Basic realm=\"Login Required\"";
static const char T_BODY[] PROGMEM = "body";
static const char T_Cache_Control[] PROGMEM = "Cache-Control";
static const char T_chunked[] PROGMEM = "chunked";
static const char T_close[] PROGMEM = "close";
static const char T_Connection[] PROGMEM = "Connection";
static const char T_Content_Disposition[] PROGMEM = "Content-Disposition";
static const char T_Content_Encoding[] PROGMEM = "Content-Encoding";
static const char T_Content_Length[] PROGMEM = "Content-Length";
static const char T_Content_Type[] PROGMEM = "Content-Type";
static const char T_Cookie[] PROGMEM = "Cookie";
static const char T_DIGEST[] PROGMEM = "Digest";
static const char T_DIGEST_[] PROGMEM = "Digest ";
static const char T_ETag[] PROGMEM = "ETag";
static const char T_EXPECT[] PROGMEM = "Expect";
static const char T_HTTP_1_0[] PROGMEM = "HTTP/1.0";
static const char T_HTTP_100_CONT[] PROGMEM = "HTTP/1.1 100 Continue\r\n\r\n";
static const char T_IMS[] PROGMEM = "If-Modified-Since";
static const char T_INM[] PROGMEM = "If-None-Match";
static const char T_keep_alive[] PROGMEM = "keep-alive";
static const char T_Last_Event_ID[] PROGMEM = "Last-Event-ID";
static const char T_Last_Modified[] PROGMEM = "Last-Modified";
static const char T_LOCATION[] PROGMEM = "Location";
static const char T_MULTIPART_[] PROGMEM = "multipart/";
static const char T_no_cache[] PROGMEM = "no-cache";
static const char T_none[] PROGMEM = "none";
static const char T_UPGRADE[] PROGMEM = "Upgrade";
static const char T_WS[] PROGMEM = "websocket";
static const char T_WWW_AUTH[] PROGMEM = "WWW-Authenticate";
static const char T_Transfer_Encoding[] PROGMEM = "Transfer-Encoding";

// HTTP Methods
static const char T_ANY[] PROGMEM = "ANY";
static const char T_GET[] PROGMEM = "GET";
static const char T_POST[] PROGMEM = "POST";
static const char T_PUT[] PROGMEM = "PUT";
static const char T_DELETE[] PROGMEM = "DELETE";
static const char T_PATCH[] PROGMEM = "PATCH";
static const char T_HEAD[] PROGMEM = "HEAD";
static const char T_OPTIONS[] PROGMEM = "OPTIONS";
static const char T_UNKNOWN[] PROGMEM = "UNKNOWN";

// Req content types
static const char T_RCT_NOT_USED[] PROGMEM = "RCT_NOT_USED";
static const char T_RCT_DEFAULT[] PROGMEM = "RCT_DEFAULT";
static const char T_RCT_HTTP[] PROGMEM = "RCT_HTTP";
static const char T_RCT_WS[] PROGMEM = "RCT_WS";
static const char T_RCT_EVENT[] PROGMEM = "RCT_EVENT";
static const char T_ERROR[] PROGMEM = "ERROR";

// extentions & MIME-Types
static const char T__css[] PROGMEM = ".css";
static const char T__eot[] PROGMEM = ".eot";
static const char T__gif[] PROGMEM = ".gif";
static const char T__gz[] PROGMEM = ".gz";
static const char T__htm[] PROGMEM = ".htm";
static const char T__html[] PROGMEM = ".html";
static const char T__ico[] PROGMEM = ".ico";
static const char T__jpg[] PROGMEM = ".jpg";
static const char T__js[] PROGMEM = ".js";
static const char T__json[] PROGMEM = ".json";
static const char T__pdf[] PROGMEM = ".pdf";
static const char T__png[] PROGMEM = ".png";
static const char T__svg[] PROGMEM = ".svg";
static const char T__ttf[] PROGMEM = ".ttf";
static const char T__woff[] PROGMEM = ".woff";
static const char T__woff2[] PROGMEM = ".woff2";
static const char T__xml[] PROGMEM = ".xml";
static const char T__zip[] PROGMEM = ".zip";
static const char T_application_javascript[] PROGMEM = "application/javascript";
static const char T_application_json[] PROGMEM = "application/json";
static const char T_application_msgpack[] PROGMEM = "application/msgpack";
static const char T_application_pdf[] PROGMEM = "application/pdf";
static const char T_application_x_gzip[] PROGMEM = "application/x-gzip";
static const char T_application_zip[] PROGMEM = "application/zip";
static const char T_font_eot[] PROGMEM = "font/eot";
static const char T_font_ttf[] PROGMEM = "font/ttf";
static const char T_font_woff[] PROGMEM = "font/woff";
static const char T_font_woff2[] PROGMEM = "font/woff2";
static const char T_image_gif[] PROGMEM = "image/gif";
static const char T_image_jpeg[] PROGMEM = "image/jpeg";
static const char T_image_png[] PROGMEM = "image/png";
static const char T_image_svg_xml[] PROGMEM = "image/svg+xml";
static const char T_image_x_icon[] PROGMEM = "image/x-icon";
static const char T_text_css[] PROGMEM = "text/css";
static const char T_text_event_stream[] PROGMEM = "text/event-stream";
static const char T_text_html[] PROGMEM = "text/html";
static const char T_text_plain[] PROGMEM = "text/plain";
static const char T_text_xml[] PROGMEM = "text/xml";

// Responce codes
static const char T_HTTP_CODE_100[] PROGMEM = "Continue";
static const char T_HTTP_CODE_101[] PROGMEM = "Switching Protocols";
static const char T_HTTP_CODE_200[] PROGMEM = "OK";
static const char T_HTTP_CODE_201[] PROGMEM = "Created";
static const char T_HTTP_CODE_202[] PROGMEM = "Accepted";
static const char T_HTTP_CODE_203[] PROGMEM = "Non-Authoritative Information";
static const char T_HTTP_CODE_204[] PROGMEM = "No Content";
static const char T_HTTP_CODE_205[] PROGMEM = "Reset Content";
static const char T_HTTP_CODE_206[] PROGMEM = "Partial Content";
static const char T_HTTP_CODE_300[] PROGMEM = "Multiple Choices";
static const char T_HTTP_CODE_301[] PROGMEM = "Moved Permanently";
static const char T_HTTP_CODE_302[] PROGMEM = "Found";
static const char T_HTTP_CODE_303[] PROGMEM = "See Other";
static const char T_HTTP_CODE_304[] PROGMEM = "Not Modified";
static const char T_HTTP_CODE_305[] PROGMEM = "Use Proxy";
static const char T_HTTP_CODE_307[] PROGMEM = "Temporary Redirect";
static const char T_HTTP_CODE_400[] PROGMEM = "Bad Request";
static const char T_HTTP_CODE_401[] PROGMEM = "Unauthorized";
static const char T_HTTP_CODE_402[] PROGMEM = "Payment Required";
static const char T_HTTP_CODE_403[] PROGMEM = "Forbidden";
static const char T_HTTP_CODE_404[] PROGMEM = "Not Found";
static const char T_HTTP_CODE_405[] PROGMEM = "Method Not Allowed";
static const char T_HTTP_CODE_406[] PROGMEM = "Not Acceptable";
static const char T_HTTP_CODE_407[] PROGMEM = "Proxy Authentication Required";
static const char T_HTTP_CODE_408[] PROGMEM = "Request Time-out";
static const char T_HTTP_CODE_409[] PROGMEM = "Conflict";
static const char T_HTTP_CODE_410[] PROGMEM = "Gone";
static const char T_HTTP_CODE_411[] PROGMEM = "Length Required";
static const char T_HTTP_CODE_412[] PROGMEM = "Precondition Failed";
static const char T_HTTP_CODE_413[] PROGMEM = "Request Entity Too Large";
static const char T_HTTP_CODE_414[] PROGMEM = "Request-URI Too Large";
static const char T_HTTP_CODE_415[] PROGMEM = "Unsupported Media Type";
static const char T_HTTP_CODE_416[] PROGMEM = "Requested range not satisfiable";
static const char T_HTTP_CODE_417[] PROGMEM = "Expectation Failed";
static const char T_HTTP_CODE_429[] PROGMEM = "Too Many Requests";
static const char T_HTTP_CODE_500[] PROGMEM = "Internal Server Error";
static const char T_HTTP_CODE_501[] PROGMEM = "Not Implemented";
static const char T_HTTP_CODE_502[] PROGMEM = "Bad Gateway";
static const char T_HTTP_CODE_503[] PROGMEM = "Service Unavailable";
static const char T_HTTP_CODE_504[] PROGMEM = "Gateway Time-out";
static const char T_HTTP_CODE_505[] PROGMEM = "HTTP Version not supported";
static const char T_HTTP_CODE_ANY[] PROGMEM = "Unknown code";

// other
static const char T__opaque[] PROGMEM = "\", opaque=\"";
static const char T_13[] PROGMEM = "13";
static const char T_asyncesp[] PROGMEM = "asyncesp";
static const char T_auth_nonce[] PROGMEM = "\", qop=\"auth\", nonce=\"";
static const char T_cnonce[] PROGMEM = "cnonce";
static const char T_data_[] PROGMEM = "data: ";
static const char T_event_[] PROGMEM = "event: ";
static const char T_filename[] PROGMEM = "filename";
static const char T_gzip[] PROGMEM = "gzip";
static const char T_Host[] PROGMEM = "Host";
static const char T_id__[] PROGMEM = "id: ";
static const char T_name[] PROGMEM = "name";
static const char T_nc[] PROGMEM = "nc";
static const char T_nonce[] PROGMEM = "nonce";
static const char T_opaque[] PROGMEM = "opaque";
static const char T_qop[] PROGMEM = "qop";
static const char T_realm[] PROGMEM = "realm";
static const char T_realm__[] PROGMEM = "realm=\"";
static const char T_response[] PROGMEM = "response";
static const char T_retry_[] PROGMEM = "retry: ";
static const char T_rn[] PROGMEM = "\r\n";
static const char T_rnrn[] PROGMEM = "\r\n\r\n";
static const char T_uri[] PROGMEM = "uri";
static const char T_username[] PROGMEM = "username";

#endif  // ESP8622

}   // namespace asyncsrv {}
