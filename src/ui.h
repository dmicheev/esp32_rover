#ifndef _UI_H
#define _UI_H

#include <Arduino.h>
#include <FS.h>

// Инициализация файловой системы
bool ui_init();

// Получение HTML страницы из файловой системы
String getUIHTML();

// Проверка наличия файла
bool ui_fileExists(const String& path);

#endif
