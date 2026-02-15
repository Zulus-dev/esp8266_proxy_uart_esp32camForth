#include "ws_handler.h"
#include "buffer_manager.h"
#include "globals.h"

extern WebSocketsServer webSocket;

void wsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  (void)num;

  switch (type) {
    case WStype_DISCONNECTED:
      break;

    case WStype_CONNECTED:
      break;

    case WStype_TEXT:  // Команды от JS -> Serial (UART)
      Serial.write(payload, len);
      if (len > 0 && payload[len - 1] != '\n') {
        Serial.write('\n');  // Добавляем \n для Forth
      }
      break;

    case WStype_BIN:  // Игнорируем binary для избежания шума в UART
      break;

    default:
      break;
  }
}

void initWSHandler() {
  webSocket.onEvent(wsEvent);
  // Нет path() — WS на корне порта 81 по умолчанию
}
