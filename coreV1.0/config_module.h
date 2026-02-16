#pragma once

#include <Arduino.h>

extern char configSSID[32];
extern char configPASS[64];
extern char configMAC[18];

void loadConfig();
void saveConfig(const char* ssid, const char* pass, const char* mac);
bool applyConfiguredSoftApMac();
