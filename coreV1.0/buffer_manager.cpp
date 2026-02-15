#include "buffer_manager.h"
#include "globals.h"

char uartRawBuffer[UART_RX_BUF_SIZE];
size_t uartRawIdx = 0;

static char lineBuffer[UART_LINE_BUF_SIZE];
static size_t lineIdx = 0;
static unsigned long lastRxAt = 0;
static const unsigned long FLUSH_TIMEOUT_MS = 30;

static inline void flushLineBuffer() {
  if (lineIdx == 0) return;

  webSocket.broadcastTXT(lineBuffer, lineIdx);
  lineIdx = 0;
}

static inline void pushToLineBuffer(char ch) {
  if (lineIdx >= UART_LINE_BUF_SIZE - 1) {
    flushLineBuffer();
  }

  lineBuffer[lineIdx++] = ch;

  if (ch == '\n') {
    flushLineBuffer();
  }
}

void processUartBuffer() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0) break;

    char ch = static_cast<char>(c);

    if (uartRawIdx < UART_RX_BUF_SIZE - 1) {
      uartRawBuffer[uartRawIdx++] = ch;
    }

    pushToLineBuffer(ch);
    lastRxAt = millis();
  }

  if (lineIdx > 0 && (millis() - lastRxAt) > FLUSH_TIMEOUT_MS) {
    flushLineBuffer();
  }

  uartRawIdx = 0;
}

void cleanupBuffers() {
  if (lineIdx > UART_LINE_BUF_SIZE - 16) {
    flushLineBuffer();
  }
}
