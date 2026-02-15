#include "uart.h"
#include "buffer_manager.h"

void handleUARTSend(AsyncWebServerRequest *request) {
  char cmd[256] = {0};

  if (request->hasParam("cmd", true)) {
    request->getParam("cmd", true)->value().toCharArray(cmd, sizeof(cmd));
  } else if (request->hasParam("cmd")) {
    request->getParam("cmd")->value().toCharArray(cmd, sizeof(cmd));
  } else {
    request->send(400, "text/plain", "No command");
    return;
  }

  size_t len = strlen(cmd);
  if (len == 0) {
    request->send(400, "text/plain", "Empty command");
    return;
  }

  Serial.write(reinterpret_cast<const uint8_t*>(cmd), len);
  if (cmd[len - 1] != '\n') {
    Serial.write('\n');
  }

  request->send(200, "text/plain", "OK");
}

void handleUARTMode(AsyncWebServerRequest *request) {
  char mode[16] = {0};

  if (!request->hasParam("mode")) {
    request->send(400, "text/plain", "Missing mode");
    return;
  }

  request->getParam("mode")->value().toCharArray(mode, sizeof(mode));

  if (strcmp(mode, "wifi") == 0) {
    Serial.println("WIFI-MODE host:port");
    request->send(200, "text/plain", "Mode switched to wifi");
    return;
  }

  if (strcmp(mode, "uart") == 0) {
    request->send(200, "text/plain", "Mode switched to uart");
    return;
  }

  request->send(400, "text/plain", "Unsupported mode");
}

void registerUARTHandlers(AsyncWebServer& server) {
  server.on("/api/uart/send", HTTP_POST, handleUARTSend);
  server.on("/api/uart/mode", HTTP_GET, handleUARTMode);
}
