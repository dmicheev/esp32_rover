#ifndef _RWIFI_H
#define _RWIFI_H

#include <Arduino.h>

// Инициализация WiFi
void wifi_init();

// Обработка WiFi событий (должна вызываться в loop)
void wifi_loop();

// Проверка подключения WiFi
bool wifi_isConnected();

// Получение IP адреса
String wifi_getIP();

#endif
