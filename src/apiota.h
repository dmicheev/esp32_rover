#ifndef _API_OTA_H
#define _API_OTA_H

#include <WebServer.h>

// Инициализация OTA маршрутов
void apiota_init(WebServer& server);

// Обработчики OTA
void handleOtaPage();
void handleOtaUpload();
void handleOtaUploadResponse();

#endif
