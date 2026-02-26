#include "ui.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED false

// ===== Вспомогательные функции =====

// Установка заголовков для отключения кэширования
static void setNoCacheHeaders(WebServer& server) {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
}

// ===== Публичные функции =====

bool ui_init() {
  Serial.println("\n=== LittleFS Initialization ===");
  unsigned long start = millis();

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS init failed, trying with format...");
    if (!LittleFS.begin(true)) {
      Serial.println("ERROR: LittleFS initialization failed");
      return false;
    }
  }

  Serial.print("LittleFS initialized in ");
  Serial.print(millis() - start);
  Serial.println(" ms");

  if (!ui_fileExists("/index.html")) {
    Serial.println("WARNING: /index.html not found!");
    return false;
  }

  Serial.println("index.html found");
  Serial.println("===============================\n");
  return true;
}

String getUIHTML() {
  File file = LittleFS.open("/index.html", "r");

  if (!file) {
    Serial.println("ERROR: Failed to open index.html");
    return String("<html><body><h1>Error: File not found</h1></body></html>");
  }

  String html = "";
  while (file.available()) {
    html += file.readString();
  }

  file.close();
  return html;
}

bool ui_fileExists(const String& path) {
  return LittleFS.exists(path);
}

void ui_serveStaticFile(WebServer& server, const String& path, const String& contentType) {
  Serial.println("[UI] Serving static file: " + path);
  
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("[UI] ERROR: File not found - " + path);
    server.send(404, "text/plain", "File not found: " + path);
    return;
  }
  
  setNoCacheHeaders(server);
  server.streamFile(file, contentType);
  file.close();
  Serial.println("[UI] File sent: " + path);
}

void ui_serveIndex(WebServer& server) {
  Serial.println("[UI] Serving index.html");
  
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("[UI] ERROR: Failed to open index.html");
    server.send(500, "text/plain", "Error: File not found");
    return;
  }
  
  setNoCacheHeaders(server);
  server.streamFile(file, "text/html");
  file.close();
  Serial.println("[UI] index.html sent");
}
