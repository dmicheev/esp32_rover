#include "ui.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

// Инициализация файловой системы LittleFS
bool ui_init() {
  Serial.println("\n=== LittleFS Initialization ===");

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("ERROR: LittleFS initialization failed");
    return false;
  }

  Serial.println("LittleFS initialized successfully");

  // Проверка наличия файла index.html
  if (!ui_fileExists("/index.html")) {
    Serial.println("WARNING: /index.html not found!");
    return false;
  }

  Serial.println("index.html found");
  Serial.println("===============================\n");
  return true;
}

// Получение HTML страницы из файловой системы
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

// Проверка наличия файла
bool ui_fileExists(const String& path) {
  return LittleFS.exists(path);
}

// Раздача статических файлов из LittleFS
void ui_serveStaticFile(WebServer& server, const String& path, const String& contentType) {
  Serial.println("[UI] Serving static file: " + path);
  
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("[UI] ERROR: File not found - " + path);
    server.send(404, "text/plain", "File not found: " + path);
    return;
  }
  
  server.streamFile(file, contentType);
  file.close();
  Serial.println("[UI] File sent: " + path);
}
