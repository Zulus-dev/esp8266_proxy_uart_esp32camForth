#pragma once

#include <ESPAsyncWebServer.h>

void registerGPIOHandlers(AsyncWebServer &server);

void handleGpioMode(AsyncWebServerRequest *request);
void handleGpioWrite(AsyncWebServerRequest *request);
