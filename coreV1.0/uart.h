// CHANGE: Добавлен /mode для переключения на ESP32 WiFi.
#pragma once
#include <ESPAsyncWebServer.h>

void registerUARTHandlers(AsyncWebServer& server);
void handleUARTSend(AsyncWebServerRequest *request);
void handleUARTMode(AsyncWebServerRequest *request);  // Новый: uart/wifi