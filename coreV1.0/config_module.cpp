#include "config_module.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

char configSSID[32] = "TP-Link-A3F2";
char configPASS[64] = "87654321";

void loadConfig() {
  if (!LittleFS.exists("/www/config.json")) return;
  File f = LittleFS.open("/www/config.json", "r");
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return;
  strlcpy(configSSID, doc["ssid"] | "TP-Link-A3F2", sizeof(configSSID));
  strlcpy(configPASS, doc["pass"] | "87654321", sizeof(configPASS));
}

void saveConfig(const char* ssid, const char* pass) {
  StaticJsonDocument<256> doc;
  doc["ssid"] = ssid;
  doc["pass"] = pass;
  File f = LittleFS.open("/www/config.json", "w");
  if (f) {
    serializeJson(doc, f);
    f.close();
  }
}