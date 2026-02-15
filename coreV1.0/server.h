#pragma once

#include <ESPAsyncWebServer.h>

const char* getContentType(const char* path);
void serveStatic(const char* path, AsyncWebServerRequest *request);