#include "system.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

static char jsonResponse[1536];

static const char* wifiModeToText(WiFiMode_t mode) {
  if (mode == WIFI_AP) return "AP";
  if (mode == WIFI_STA) return "STA";
  if (mode == WIFI_AP_STA) return "AP_STA";
  return "OFF";
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

void registerSystemHandlers(AsyncWebServer &server) {
  server.on("/api/system/status", HTTP_GET, handleSystemStatus);
  server.on("/api/system/restart", HTTP_POST, handleSystemRestart);
}
