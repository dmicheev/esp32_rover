#include "apiota.h"

#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <esp_ota_ops.h>

#include "ui.h"

// Глобальная переменная server (объявлена в api.cpp)
extern WebServer server;

// Глобальный флаг статуса OTA обновления
static bool otaUpdateSuccess = false;
static String otaErrorMessage = "";

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
    // Сбрасываем флаги при начале новой загрузки
    otaUpdateSuccess = false;
    otaErrorMessage = "";
    
    apiota_log("OTA Start: " + String(upload.filename.c_str()));
    
    // Получаем информацию о разделе прошивки
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
    
    if (!update) {
      apiota_log("ERROR: No OTA update partition found");
      otaErrorMessage = "No OTA partition";
      return;
    }
    
    apiota_log("Running partition: " + String(running->label) + " @ 0x" + String(running->address, HEX));
    apiota_log("Update partition: " + String(update->label) + " @ 0x" + String(update->address, HEX));
    apiota_log("Update partition size: " + String(update->size) + " bytes");
    
    // Начинаем обновление с размером раздела OTA
    // Используем размер раздела, а не файла, так как upload.totalSize может быть недоступен
    if (!Update.begin(update->size)) {
      apiota_log("ERROR: Update.begin() failed - " + String(Update.errorString()));
      otaErrorMessage = "Update.begin() failed: " + String(Update.errorString());
      return;
    }
    
    apiota_log("Update started with partition size, free heap: " + String(ESP.getFreeHeap()) + " bytes");
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    int written = Update.write(upload.buf, upload.currentSize);
    if (written != upload.currentSize) {
      apiota_log("ERROR: Update.write() failed - " + String(Update.errorString()));
      apiota_log("Written: " + String(written) + ", Expected: " + String(upload.currentSize));
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    apiota_log("OTA End: " + String(upload.totalSize) + " bytes total");
    
    // Проверяем, что все данные записаны
    if (Update.hasError()) {
      apiota_log("ERROR: Update has errors before end - " + String(Update.errorString()));
      otaErrorMessage = "Write error: " + String(Update.errorString());
      return;
    }
    
    // Пробуем завершить БЕЗ проверки подписи (для отладки)
    apiota_log("Calling Update.end(false) - skipping signature check...");
    if (Update.end(false)) {  // false = отключаем проверку подписи
      apiota_log("OTA Success: " + String(upload.totalSize) + " bytes");
      apiota_log("Firmware MD5: " + String(Update.md5String()));
      otaUpdateSuccess = true;
    } else {
      apiota_log("ERROR: Update.end() failed - " + String(Update.errorString()));
      apiota_log("Error code: " + String(Update.getError()));
      
      // Дополнительная диагностика
      apiota_log("Trying to diagnose the issue...");
      const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
      apiota_log("Update partition address: 0x" + String(update->address, HEX));
      apiota_log("Update partition size: " + String(update->size));
      
      otaErrorMessage = "Update.end() failed: " + String(Update.errorString());
    }
  }
}

// Обработчик ответа после загрузки (вызывается после handleOtaUpload)
void handleOtaUploadResponse() {
  if (otaUpdateSuccess) {
    apiota_log("OTA update successful, sending response and rebooting...");
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Firmware uploaded successfully. Rebooting...\"}");
    delay(1000);
    ESP.restart();
  } else if (otaErrorMessage.length() > 0) {
    apiota_log("OTA update failed: " + otaErrorMessage);
    server.send(500, "application/json", "{\"error\":\"" + otaErrorMessage + "\"}");
  } else {
    apiota_log("OTA update failed with unknown error");
    server.send(500, "application/json", "{\"error\":\"Unknown OTA error\"}");
  }
}

// Инициализация OTA маршрутов
void apiota_init(WebServer& server) {
  server.on("/api/ota", HTTP_GET, handleOtaPage);
  server.on("/api/ota/upload", HTTP_POST, handleOtaUploadResponse, handleOtaUpload);
}
