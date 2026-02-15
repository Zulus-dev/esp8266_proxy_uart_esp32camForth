#include "json_utils.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

void sendJsonStatus(AsyncWebServerRequest *request, FSInfo& info, uint32_t heapValue) {
  StaticJsonDocument<512> doc;
  doc["heap"] = heapValue;

  JsonObject fs = doc.createNestedObject("fs");
  fs["totalKB"] = info.totalBytes / 1024;
  fs["usedKB"] = info.usedBytes / 1024;

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["mode"] = (WiFi.getMode() & WIFI_AP) ? "AP" : "STA";
  wifi["status"] = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
  wifi["rssi"] = WiFi.RSSI();

  char out[512];
  serializeJson(doc, out, sizeof(out));
  request->send(200, "application/json", out);
}
