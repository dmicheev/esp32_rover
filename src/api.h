#ifndef _API_H
#define _API_H

#include <Arduino.h>

// Инициализация API сервера
void api_init();

// Обработка клиентских запросов (должна вызываться в loop)
void api_loop();

#endif
