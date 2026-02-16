#include "config_module.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

extern "C" {
  #include <user_interface.h>
}

char configSSID[32] = "TP-Link-A3F2";
char configPASS[64] = "87654321";
char configMAC[18] = "";

static bool parseMacText(const char* text, uint8_t* out) {
  if (!text) return false;
  if (strlen(text) != 17) return false;

  for (int i = 0; i < 6; i++) {
    char high = text[i * 3];
    char low = text[i * 3 + 1];

    if (i < 5 && text[i * 3 + 2] != ':') return false;

    auto hexVal = [](char ch) -> int {
      if (ch >= '0' && ch <= '9') return ch - '0';
      if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
      if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
      return -1;
    };

    int hi = hexVal(high);
    int lo = hexVal(low);
    if (hi < 0 || lo < 0) return false;

    out[i] = static_cast<uint8_t>((hi << 4) | lo);
  }

  return true;
}

void loadConfig() {
  if (!LittleFS.exists("/www/config.json")) return;

  File f = LittleFS.open("/www/config.json", "r");
  if (!f) return;

  StaticJsonDocument<320> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return;

  strlcpy(configSSID, doc["ssid"] | "TP-Link-A3F2", sizeof(configSSID));
  strlcpy(configPASS, doc["pass"] | "87654321", sizeof(configPASS));
  strlcpy(configMAC, doc["mac"] | "", sizeof(configMAC));
}

void saveConfig(const char* ssid, const char* pass, const char* mac) {
  StaticJsonDocument<320> doc;
  doc["ssid"] = ssid;
  doc["pass"] = pass;
  doc["mac"] = mac ? mac : "";

  File f = LittleFS.open("/www/config.json", "w");
  if (!f) return;

  serializeJson(doc, f);
  f.close();
}

bool applyConfiguredSoftApMac() {
  if (configMAC[0] == '\0') return false;

  uint8_t mac[6];
  if (!parseMacText(configMAC, mac)) return false;

  return wifi_set_macaddr(SOFTAP_IF, mac);
}
