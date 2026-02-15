#pragma once

#include <Arduino.h>

#define UART_RX_BUF_SIZE 2048
#define UART_LINE_BUF_SIZE 512

extern char uartRawBuffer[UART_RX_BUF_SIZE];
extern size_t uartRawIdx;

void processUartBuffer();
void cleanupBuffers();
