#include "server.h"
#include <LittleFS.h>

const char* getContentType(const char* path) {
  if (strstr_P(path, PSTR(".html")))  return "text/html";
  if (strstr_P(path, PSTR(".css")))   return "text/css";
  if (strstr_P(path, PSTR(".js")))    return "application/javascript";
  if (strstr_P(path, PSTR(".json")))  return "application/json";
  if (strstr_P(path, PSTR(".png")))   return "image/png";
  if (strstr_P(path, PSTR(".jpg")))   return "image/jpeg";
  if (strstr_P(path, PSTR(".ico")))   return "image/x-icon";
  return "text/plain";
}

static bool isSafeStaticPath(const char* path) {
  return path && path[0] == '/' && strstr(path, "..") == nullptr;
}

void serveStatic(const char* path, AsyncWebServerRequest *request) {
  if (!path || strcmp(path, "/") == 0 || path[0] == '\0') {
    path = "/www/index.html";
  }

  if (!isSafeStaticPath(path)) {
    request->send(400, "text/plain", "Bad path");
    return;
  }

  if (!LittleFS.exists(path)) {
    request->send(404, "text/plain", "Not Found");
    return;
  }

  AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, getContentType(path));
  response->addHeader("Cache-Control", "public,max-age=300");
  request->send(response);
}
