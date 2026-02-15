#include "fs.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

static char pathBuffer[320];

static bool isSafePath(const char* path) {
  if (!path || path[0] != '/') return false;
  if (strstr(path, "..")) return false;
  return true;
}

static bool getPathParam(AsyncWebServerRequest *request, const char* name, char* out, size_t outSize, bool post = false) {
  if (!request->hasParam(name, post)) return false;

  String value = request->getParam(name, post)->value();
  if (value.length() == 0 || value.length() >= outSize) return false;

  value.toCharArray(out, outSize);
  return isSafePath(out);
}

void handleFSList(AsyncWebServerRequest *request) {
  const char *dir = "/";

  if (request->hasParam("dir")) {
    if (!getPathParam(request, "dir", pathBuffer, sizeof(pathBuffer))) {
      request->send(400, "application/json", "{\"error\":\"invalid dir\"}");
      return;
    }
    dir = pathBuffer;
  }

  Dir dirObj = LittleFS.openDir(dir);
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print('[');

  bool first = true;
  while (dirObj.next()) {
    if (!first) response->print(',');
    first = false;

    response->print("{\"name\":\"");
    response->print(dirObj.fileName());
    response->print("\",\"size\":");
    response->print(dirObj.fileSize());
    response->print(",\"isDir\":");
    response->print(dirObj.isDirectory() ? "true" : "false");
    response->print("}");
  }

  response->print(']');
  request->send(response);
}

void handleFSRead(AsyncWebServerRequest *request) {
  if (!getPathParam(request, "file", pathBuffer, sizeof(pathBuffer))) {
    request->send(400, "text/plain", "Parameter 'file' is invalid");
    return;
  }

  if (!LittleFS.exists(pathBuffer)) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  request->send(LittleFS, pathBuffer, "application/octet-stream");
}

void handleFSDelete(AsyncWebServerRequest *request) {
  if (!getPathParam(request, "file", pathBuffer, sizeof(pathBuffer), true)) {
    request->send(400, "text/plain", "Parameter 'file' is invalid");
    return;
  }

  bool removeDir = request->hasParam("dir", true) && request->getParam("dir", true)->value() == "1";

  if (!LittleFS.exists(pathBuffer)) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  bool ok = false;
  if (removeDir) {
    ok = LittleFS.rmdir(pathBuffer);
  } else {
    ok = LittleFS.remove(pathBuffer);
  }

  request->send(ok ? 200 : 500, "text/plain", ok ? "Deleted" : "Delete failed");
}

void handleFSStat(AsyncWebServerRequest *request) {
  if (!getPathParam(request, "file", pathBuffer, sizeof(pathBuffer))) {
    request->send(400, "application/json", "{\"error\":\"invalid file\"}");
    return;
  }

  if (!LittleFS.exists(pathBuffer)) {
    request->send(404, "application/json", "{\"error\":\"not found\"}");
    return;
  }

  File f = LittleFS.open(pathBuffer, "r");
  if (!f) {
    request->send(500, "application/json", "{\"error\":\"open failed\"}");
    return;
  }

  StaticJsonDocument<192> doc;
  doc["name"] = pathBuffer;
  doc["size"] = f.size();
  f.close();

  char out[192];
  serializeJson(doc, out, sizeof(out));
  request->send(200, "application/json", out);
}

void handleFSMove(AsyncWebServerRequest *request) {
  char src[320];
  char dst[320];

  if (!getPathParam(request, "src", src, sizeof(src), true) || !getPathParam(request, "dst", dst, sizeof(dst), true)) {
    request->send(400, "text/plain", "Invalid src/dst");
    return;
  }

  if (!LittleFS.exists(src)) {
    request->send(404, "text/plain", "Source not found");
    return;
  }

  bool ok = LittleFS.rename(src, dst);
  request->send(ok ? 200 : 500, "text/plain", ok ? "Moved" : "Move failed");
}

void handleFSMkdir(AsyncWebServerRequest *request) {
  if (!getPathParam(request, "dir", pathBuffer, sizeof(pathBuffer), true)) {
    request->send(400, "text/plain", "Invalid dir");
    return;
  }

  bool ok = LittleFS.mkdir(pathBuffer);
  request->send(ok ? 200 : 500, "text/plain", ok ? "Directory created" : "mkdir failed");
}

void handleFSCopy(AsyncWebServerRequest *request) {
  char src[320];
  char dst[320];

  if (!getPathParam(request, "src", src, sizeof(src), true) || !getPathParam(request, "dst", dst, sizeof(dst), true)) {
    request->send(400, "text/plain", "Invalid src/dst");
    return;
  }

  if (!LittleFS.exists(src)) {
    request->send(404, "text/plain", "Source not found");
    return;
  }

  File in = LittleFS.open(src, "r");
  if (!in) {
    request->send(500, "text/plain", "Cannot open source");
    return;
  }

  File out = LittleFS.open(dst, "w");
  if (!out) {
    in.close();
    request->send(500, "text/plain", "Cannot open destination");
    return;
  }

  uint8_t buf[256];
  while (in.available()) {
    size_t n = in.read(buf, sizeof(buf));
    if (n == 0) break;
    if (out.write(buf, n) != n) {
      in.close();
      out.close();
      request->send(500, "text/plain", "Copy write failed");
      return;
    }
  }

  in.close();
  out.close();
  request->send(200, "text/plain", "Copied");
}

void handleFSWriteText(AsyncWebServerRequest *request) {
  char filePath[320];
  if (!getPathParam(request, "file", filePath, sizeof(filePath), true)) {
    request->send(400, "text/plain", "Invalid file");
    return;
  }

  if (!request->hasParam("content", true)) {
    request->send(400, "text/plain", "Missing content");
    return;
  }

  String content = request->getParam("content", true)->value();

  File out = LittleFS.open(filePath, "w");
  if (!out) {
    request->send(500, "text/plain", "Cannot open file");
    return;
  }

  size_t written = out.print(content);
  out.close();

  request->send((written == content.length()) ? 200 : 500, "text/plain", (written == content.length()) ? "Saved" : "Partial save");
}

static File uploadFile;
static bool uploadFailed = false;

void handleFSWrite(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (!index) {
    uploadFailed = false;

    if (!getPathParam(request, "file", pathBuffer, sizeof(pathBuffer), true)) {
      uploadFailed = true;
      return;
    }

    if (uploadFile) uploadFile.close();
    uploadFile = LittleFS.open(pathBuffer, "w");

    if (!uploadFile) {
      uploadFailed = true;
      return;
    }
  }

  if (!uploadFailed && uploadFile && len) {
    if (uploadFile.write(data, len) != len) {
      uploadFailed = true;
    }
  }

  if (index + len == total) {
    if (uploadFile) uploadFile.close();
  }
}

void registerFSHandlers(AsyncWebServer &server) {
  server.on("/api/fs/list",      HTTP_GET,  handleFSList);
  server.on("/api/fs/read",      HTTP_GET,  handleFSRead);
  server.on("/api/fs/stat",      HTTP_GET,  handleFSStat);
  server.on("/api/fs/delete",    HTTP_POST, handleFSDelete);
  server.on("/api/fs/move",      HTTP_POST, handleFSMove);
  server.on("/api/fs/mkdir",     HTTP_POST, handleFSMkdir);
  server.on("/api/fs/copy",      HTTP_POST, handleFSCopy);
  server.on("/api/fs/writeText", HTTP_POST, handleFSWriteText);

  server.on(
    "/api/fs/write",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
      if (uploadFailed) {
        request->send(500, "text/plain", "Upload failed");
      } else {
        request->send(200, "text/plain", "File written successfully");
      }
    },
    NULL,
    handleFSWrite
  );
}
