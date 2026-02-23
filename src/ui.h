#ifndef _UI_H
#define _UI_H

#include <Arduino.h>
#include <FS.h>
#include <WebServer.h>

// Инициализация файловой системы
bool ui_init();

// Получение HTML страницы из файловой системы (устарело, использовать ui_serveIndex)
String getUIHTML();

// Проверка наличия файла
bool ui_fileExists(const String& path);

// Раздача статических файлов из LittleFS
void ui_serveStaticFile(WebServer& server, const String& path, const String& contentType);

// Раздача главной страницы (потоковая)
void ui_serveIndex(WebServer& server);

#endif
