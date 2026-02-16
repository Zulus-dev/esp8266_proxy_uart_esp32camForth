#include "system.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config_module.h"

extern "C" {
  #include <user_interface.h>
}

static char jsonResponse[1536];

static const char* wifiModeToText(WiFiMode_t mode) {
  if (mode == WIFI_AP) return "AP";
  if (mode == WIFI_STA) return "STA";
  if (mode == WIFI_AP_STA) return "AP_STA";
  return "OFF";
}

static bool parseMac(const String& macText, uint8_t* out) {
  if (macText.length() != 17) return false;

  for (int i = 0; i < 6; i++) {
    char high = macText[i * 3];
    char low = macText[i * 3 + 1];
    if (i < 5 && macText[i * 3 + 2] != ':') return false;

    auto hexVal = [](char ch) -> int {
      if (ch >= '0' && ch <= '9') return ch - '0';
      if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
      if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
      return -1;
    };

    int hi = hexVal(high);
    int lo = hexVal(low);
    if (hi < 0 || lo < 0) return false;
    out[i] = (uint8_t)((hi << 4) | lo);
  }

  return true;
}

void handleSystemStatus(AsyncWebServerRequest *request) {
  FSInfo fsinfo;
  LittleFS.info(fsinfo);

  StaticJsonDocument<1536> doc;
  doc["uptime"] = millis() / 1000UL;

  JsonObject heap = doc.createNestedObject("heap");
  heap["free"] = ESP.getFreeHeap();
  heap["frag"] = ESP.getHeapFragmentation();
  heap["maxBlock"] = ESP.getMaxFreeBlockSize();

  JsonObject fs = doc.createNestedObject("fs");
  fs["total"] = fsinfo.totalBytes;
  fs["used"] = fsinfo.usedBytes;
  fs["free"] = fsinfo.totalBytes - fsinfo.usedBytes;

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["mode"] = wifiModeToText(WiFi.getMode());
  wifi["status"] = (WiFi.status() == WL_CONNECTED) ? "connected" : "not_connected";
  wifi["ap_ip"] = WiFi.softAPIP().toString();
  wifi["sta_ip"] = WiFi.localIP().toString();
  wifi["rssi"] = WiFi.RSSI();
  wifi["ap_clients"] = WiFi.softAPgetStationNum();

  JsonObject chip = doc.createNestedObject("chip");
  chip["id"] = ESP.getChipId();
  chip["cpu_mhz"] = ESP.getCpuFreqMHz();
  chip["sdk"] = ESP.getSdkVersion();
  chip["reset_reason"] = ESP.getResetReason();

  JsonObject flash = doc.createNestedObject("flash");
  flash["real_size"] = ESP.getFlashChipRealSize();
  flash["sketch_size"] = ESP.getSketchSize();
  flash["free_space"] = ESP.getFreeSketchSpace();

  size_t len = serializeJson(doc, jsonResponse, sizeof(jsonResponse));
  if (len >= sizeof(jsonResponse) - 1) {
    request->send(500, "application/json", "{\"error\":\"status buffer too small\"}");
    return;
  }

  request->send(200, "application/json", jsonResponse);
}

void handleSystemRestart(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Restarting...");
  delay(400);
  ESP.restart();
}

void handleSettingsGet(AsyncWebServerRequest *request) {
  StaticJsonDocument<256> doc;
  doc["ssid"] = configSSID;
  doc["pass"] = configPASS;
  doc["ap_mac"] = WiFi.softAPmacAddress();

  char out[256];
  serializeJson(doc, out, sizeof(out));
  request->send(200, "application/json", out);
}

void handleSettingsSave(AsyncWebServerRequest *request) {
  if (!request->hasParam("ssid", true) || !request->hasParam("pass", true)) {
    request->send(400, "application/json", "{\"error\":\"ssid and pass are required\"}");
    return;
  }

  String ssid = request->getParam("ssid", true)->value();
  String pass = request->getParam("pass", true)->value();

  if (ssid.length() < 1 || ssid.length() >= sizeof(configSSID)) {
    request->send(400, "application/json", "{\"error\":\"invalid ssid length\"}");
    return;
  }

  if (pass.length() >= sizeof(configPASS)) {
    request->send(400, "application/json", "{\"error\":\"invalid password length\"}");
    return;
  }

  strlcpy(configSSID, ssid.c_str(), sizeof(configSSID));
  strlcpy(configPASS, pass.c_str(), sizeof(configPASS));
  saveConfig(configSSID, configPASS);

  bool macApplied = false;
  String macInput;
  if (request->hasParam("mac", true)) {
    macInput = request->getParam("mac", true)->value();
    macInput.trim();

    if (macInput.length() > 0) {
      uint8_t macBytes[6];
      if (!parseMac(macInput, macBytes)) {
        request->send(400, "application/json", "{\"error\":\"invalid MAC format (AA:BB:CC:DD:EE:FF)\"}");
        return;
      }

      macApplied = wifi_set_macaddr(SOFTAP_IF, macBytes);
    }
  }

  WiFi.softAPdisconnect(true);
  WiFi.softAP(configSSID, configPASS);

  StaticJsonDocument<320> doc;
  doc["ok"] = true;
  doc["ssid"] = configSSID;
  doc["ap_mac"] = WiFi.softAPmacAddress();
  doc["mac_applied"] = macApplied;

  char out[320];
  serializeJson(doc, out, sizeof(out));
  request->send(200, "application/json", out);
}

void registerSystemHandlers(AsyncWebServer &server) {
  server.on("/api/system/status", HTTP_GET, handleSystemStatus);
  server.on("/api/system/restart", HTTP_POST, handleSystemRestart);
  server.on("/api/settings", HTTP_GET, handleSettingsGet);
  server.on("/api/settings", HTTP_POST, handleSettingsSave);
}
