#include "apiota.h"

#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "ui.h"

// Глобальная переменная server (объявлена в api.cpp)
extern WebServer server;

// Логирование
void apiota_log(const String& message) {
  Serial.println("[OTA] " + message);
}

// GET /api/ota - страница загрузки прошивки
void handleOtaPage() {
  apiota_log("GET /api/ota (OTA page)");
  
  // Проверяем наличие файла
  if (!ui_fileExists("/ota.html")) {
    Serial.println("ERROR: ota.html not found");
    return;
  }
  
  // Отправляем файл напрямую из LittleFS
  File file = LittleFS.open("/ota.html", "r");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  }
}

// POST /api/ota/upload - загрузка прошивки
void handleOtaUpload() {
  apiota_log("POST /api/ota/upload");
  
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    apiota_log("OTA Start: " + String(upload.filename.c_str()));
    
    // Проверка размера файла
    size_t freeSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (upload.totalSize > freeSpace) {
      apiota_log("ERROR: Not enough space. Free: " + String(freeSpace) + ", Need: " + String(upload.totalSize));
      server.send(507, "application/json", "{\"error\":\"Not enough space\"}");
      return;
    }
    
    if (!Update.begin(freeSpace)) {
      apiota_log("ERROR: Update.begin() failed - " + String(Update.errorString()));
      server.send(500, "application/json", "{\"error\":\"Update.begin() failed\"}");
      return;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      apiota_log("ERROR: Update.write() failed - " + String(Update.errorString()));
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      apiota_log("OTA Success: " + String(upload.totalSize) + " bytes");
      server.sendHeader("Connection", "close");
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Firmware uploaded successfully. Rebooting...\"}");
      delay(1000);
      ESP.restart();
    } else {
      apiota_log("ERROR: Update.end() failed - " + String(Update.errorString()));
      server.send(500, "application/json", "{\"error\":\"Update.end() failed\"}");
    }
  }
}

// Инициализация OTA маршрутов
void apiota_init(WebServer& server) {
  server.on("/api/ota", HTTP_GET, handleOtaPage);
  server.on("/api/ota/upload", HTTP_POST, handleOtaUpload);
}
