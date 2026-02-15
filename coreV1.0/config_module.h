// CHANGE: Новый модуль. Фиксированные буферы вместо String.
#pragma once
#include <Arduino.h>

extern char configSSID[32];
extern char configPASS[64];
void loadConfig();
void saveConfig(const char* ssid, const char* pass);