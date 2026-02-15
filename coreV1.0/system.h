#pragma once

#include <ESPAsyncWebServer.h>

void registerSystemHandlers(AsyncWebServer &server);

void handleSystemStatus(AsyncWebServerRequest *request);
void handleSystemRestart(AsyncWebServerRequest *request);
