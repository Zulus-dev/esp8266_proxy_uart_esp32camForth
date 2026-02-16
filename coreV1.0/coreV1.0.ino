// CHANGE: Переименован в core.ino для модульности. Использует AsyncWebServer для асинхронности.
// Добавлена инициализация utils и api. Минимальный loop() с yield().
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

#include "server.h"                         // ← теперь подключаем!
#include "config_module.h"  // CHANGE: Модуль конфигурации
#include "fs.h"
#include "uart.h"
#include "system.h"
#include "gpio.h"
#include "buffer_manager.h"  // CHANGE: Новый модуль буферов
#include "ws_handler.h"   // CHANGE: Модуль WS
#include "globals.h"         

AsyncWebServer server(80);         // CHANGE: Async для избежания блокировок
WebSocketsServer webSocket(81);

void setup() {
  Serial.begin(115200);
  delay(100);
  LittleFS.begin();

  loadConfig();
  WiFi.mode(WIFI_AP);
  applyConfiguredSoftApMac();
  WiFi.softAP(configSSID, configPASS);
  registerFSHandlers(server);
  registerUARTHandlers(server);
  registerSystemHandlers(server);
  registerGPIOHandlers(server);

  server.onNotFound([](AsyncWebServerRequest *request) {
    const char* path = request->url().c_str();
    char buf[256];
    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    if (strcmp(buf, "/") == 0 || strlen(buf) == 0) {
      strcpy(buf, "/www/index.html");
    }
    serveStatic(buf, request);
  });

  server.begin();
  initWSHandler();
  webSocket.begin();  // Порт 81, путь "/" по умолчанию

}

void loop() {
  webSocket.loop();
  processUartBuffer();
  cleanupBuffers();
  yield();
}