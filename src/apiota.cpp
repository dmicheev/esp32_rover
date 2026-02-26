#include "apiota.h"

#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <esp_ota_ops.h>

#include "ui.h"

// ===== Константы =====

#define OTA_REBOOT_DELAY_MS 1000
#define OTA_MAX_FILE_SIZE 6553600  // Максимальный размер файла для OTA (6.25 MB)

// ===== Глобальные переменные =====

extern WebServer server;
static bool otaUpdateSuccess = false;
static String otaErrorMessage = "";
static unsigned long otaStartTime = 0;
static size_t otaTotalBytesWritten = 0;
static int otaChunkCount = 0;

// ===== Вспомогательные функции =====

void apiota_log(const String& message) {
  Serial.println("[OTA] " + message);
}

static void logPartitionInfo(const esp_partition_t* partition, const String& name) {
  apiota_log(name + " partition: " + String(partition->label) + 
             " @ 0x" + String(partition->address, HEX) + 
             ", size: " + String(partition->size) + " bytes");
}

static void sendOTAError(const String& message) {
  server.send(500, "application/json", "{\"error\":\"" + message + "\"}");
}

static void sendOTASuccess() {
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", 
               "{\"success\":true,\"message\":\"Firmware uploaded successfully. Rebooting...\"}");
}

// ===== Обработчики =====

void handleOtaPage() {
  apiota_log("GET /api/ota (OTA page)");

  if (!ui_fileExists("/ota.html")) {
    apiota_log("ERROR: ota.html not found");
    server.send(404, "text/plain", "OTA page not found");
    return;
  }

  File file = LittleFS.open("/ota.html", "r");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  }
}

void handleOtaUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    otaUpdateSuccess = false;
    otaErrorMessage = "";
    otaStartTime = millis();
    otaTotalBytesWritten = 0;
    otaChunkCount = 0;
    
    apiota_log("POST /api/ota/upload");
    apiota_log("OTA Start: " + String(upload.filename.c_str()));
    apiota_log("Content length: " + String(upload.totalSize) + " bytes");
    apiota_log("Free heap before OTA: " + String(ESP.getFreeHeap()) + " bytes");
    apiota_log("Free PSRAM: " + String(ESP.getPsramSize()) + " bytes");
    
    // Проверка размера файла
    if (upload.totalSize > OTA_MAX_FILE_SIZE) {
      apiota_log("ERROR: File too large: " + String(upload.totalSize) + " bytes (max: " + String(OTA_MAX_FILE_SIZE) + ")");
      otaErrorMessage = "File too large";
      return;
    }
    
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
    
    if (!update) {
      apiota_log("ERROR: No OTA update partition found");
      otaErrorMessage = "No OTA partition";
      return;
    }
    
    logPartitionInfo(running, "Running");
    logPartitionInfo(update, "Update");
    
    apiota_log("Update partition size: " + String(update->size) + " bytes");
    apiota_log("Update started with partition size, free heap: " + String(ESP.getFreeHeap()) + " bytes");
    
    // Используем размер файла вместо размера раздела для Update.begin()
    // Это может помочь избежать проблем с большими разделами
    size_t updateSize = (upload.totalSize > 0) ? upload.totalSize : update->size;
    apiota_log("Using update size: " + String(updateSize) + " bytes");
    
    if (!Update.begin(updateSize)) {
      apiota_log("ERROR: Update.begin() failed - " + String(Update.errorString()));
      apiota_log("Error code: " + String(Update.getError()));
      otaErrorMessage = "Update.begin() failed: " + String(Update.errorString());
      return;
    }
    
    apiota_log("Update.begin() successful");
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    otaChunkCount++;
    int written = Update.write(upload.buf, upload.currentSize);
    otaTotalBytesWritten += written;
    
    // Логируем каждые 50 чанков или при ошибке
    if (otaChunkCount % 50 == 0 || written != upload.currentSize) {
      apiota_log("Chunk #" + String(otaChunkCount) + 
                 ": received=" + String(upload.currentSize) + 
                 ", written=" + String(written) + 
                 ", total=" + String(otaTotalBytesWritten) + 
                 "/" + String(upload.totalSize) + " bytes");
    }
    
    if (written != upload.currentSize) {
      apiota_log("ERROR: Update.write() failed - " + String(Update.errorString()));
      apiota_log("Written: " + String(written) + ", Expected: " + String(upload.currentSize));
      apiota_log("Error code: " + String(Update.getError()));
      otaErrorMessage = "Update.write() failed: " + String(Update.errorString());
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    unsigned long elapsedTime = millis() - otaStartTime;
    float speedKBps = (otaTotalBytesWritten / 1024.0) / (elapsedTime / 1000.0);
    
    apiota_log("OTA End: " + String(upload.totalSize) + " bytes total");
    apiota_log("Total written: " + String(otaTotalBytesWritten) + " bytes");
    apiota_log("Chunks processed: " + String(otaChunkCount));
    apiota_log("Time elapsed: " + String(elapsedTime) + " ms");
    apiota_log("Average speed: " + String(speedKBps, 2) + " KB/s");
    apiota_log("Free heap after OTA: " + String(ESP.getFreeHeap()) + " bytes");
    
    if (Update.hasError()) {
      apiota_log("ERROR: Update has errors - " + String(Update.errorString()));
      apiota_log("Error code: " + String(Update.getError()));
      otaErrorMessage = "Write error: " + String(Update.errorString());
      return;
    }
    
    apiota_log("Calling Update.end(true) - skipping size check...");
    if (Update.end(true)) {
      apiota_log("OTA Success: " + String(upload.totalSize) + " bytes");
      apiota_log("Firmware MD5: " + String(Update.md5String()));
      otaUpdateSuccess = true;
    } else {
      apiota_log("ERROR: Update.end() failed - " + String(Update.errorString()));
      apiota_log("Error code: " + String(Update.getError()));
      
      const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
      apiota_log("Update partition address: 0x" + String(update->address, HEX));
      apiota_log("Update partition size: " + String(update->size));
      
      otaErrorMessage = "Update.end() failed: " + String(Update.errorString());
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    apiota_log("ERROR: Upload aborted");
    apiota_log("Total written before abort: " + String(otaTotalBytesWritten) + " bytes");
    otaErrorMessage = "Upload aborted";
  }
}

void handleOtaUploadResponse() {
  if (otaUpdateSuccess) {
    apiota_log("OTA update successful, sending response and rebooting...");
    apiota_log("Total time: " + String(millis() - otaStartTime) + " ms");
    apiota_log("Total bytes written: " + String(otaTotalBytesWritten));
    sendOTASuccess();
    delay(OTA_REBOOT_DELAY_MS);
    ESP.restart();
  } else if (otaErrorMessage.length() > 0) {
    apiota_log("OTA update failed: " + otaErrorMessage);
    apiota_log("Trying to diagnose the issue...");
    
    // Дополнительная диагностика
    const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
    if (update) {
      apiota_log("Update partition address: 0x" + String(update->address, HEX));
      apiota_log("Update partition size: " + String(update->size));
    }
    
    apiota_log("OTA update failed: Update.end() failed: " + otaErrorMessage);
    sendOTAError(otaErrorMessage);
  } else {
    apiota_log("OTA update failed with unknown error");
    sendOTAError("Unknown OTA error");
  }
}

void apiota_init(WebServer& server) {
  server.on("/api/ota", HTTP_GET, handleOtaPage);
  server.on("/api/ota/upload", HTTP_POST, handleOtaUploadResponse, handleOtaUpload);
}
