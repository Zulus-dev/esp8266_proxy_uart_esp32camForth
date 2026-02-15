#pragma once

#include <ESPAsyncWebServer.h>

void registerFSHandlers(AsyncWebServer& server);
void handleFSList(AsyncWebServerRequest *request);
void handleFSRead(AsyncWebServerRequest *request);
void handleFSDelete(AsyncWebServerRequest *request);
void handleFSWrite(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleFSStat(AsyncWebServerRequest *request);
void handleFSMove(AsyncWebServerRequest *request);
void handleFSMkdir(AsyncWebServerRequest *request);
void handleFSCopy(AsyncWebServerRequest *request);
void handleFSWriteText(AsyncWebServerRequest *request);
