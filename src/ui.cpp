#include "ui.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED false  // Не форматировать автоматически - это долго!

// Инициализация файловой системы LittleFS
bool ui_init() {
  Serial.println("\n=== LittleFS Initialization ===");
  unsigned long start = millis();

  // Пробуем инициализировать без форматирования (быстрее)
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS init failed, trying with format...");
    // Если не вышло - пробуем с форматированием
    if (!LittleFS.begin(true)) {
      Serial.println("ERROR: LittleFS initialization failed");
      return false;
    }
  }

  Serial.print("LittleFS initialized in ");
  Serial.print(millis() - start);
  Serial.println(" ms");

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

// Раздача статических файлов из LittleFS с кэшированием
void ui_serveStaticFile(WebServer& server, const String& path, const String& contentType) {
  Serial.println("[UI] Serving static file: " + path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("[UI] ERROR: File not found - " + path);
    server.send(404, "text/plain", "File not found: " + path);
    return;
  }

  // Добавляем заголовки для кэширования
  server.sendHeader("Cache-Control", "max-age=86400");  // Кэшировать 24 часа
  server.sendHeader("ETag", "esp32-static-" + path);

  server.streamFile(file, contentType);
  file.close();
  Serial.println("[UI] File sent: " + path);
}

// Раздача главной страницы (потоковая) с кэшированием
void ui_serveIndex(WebServer& server) {
  Serial.println("[UI] Serving index.html");

  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("[UI] ERROR: Failed to open index.html");
    server.send(500, "text/plain", "Error: File not found");
    return;
  }

  // Добавляем заголовки для кэширования
  server.sendHeader("Cache-Control", "max-age=3600");  // Кэшировать 1 час
  server.sendHeader("ETag", "esp32-robot-ui-v1");

  server.streamFile(file, "text/html");
  file.close();
  Serial.println("[UI] index.html sent");
}
