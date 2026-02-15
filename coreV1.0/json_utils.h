#pragma once

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

void sendJsonStatus(AsyncWebServerRequest *request, FSInfo& info, uint32_t heap);
