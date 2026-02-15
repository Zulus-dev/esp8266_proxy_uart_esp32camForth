#pragma once
#include <WebSocketsServer.h>

void initWSHandler();
void wsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len);