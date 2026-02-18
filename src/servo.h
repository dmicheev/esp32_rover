#ifndef _SERVO_H
#define _SERVO_H

#include <stdint.h>

// Инициализация сервоприводов
void setup_serv();

// Обработка команд Serial (должна вызываться в loop)
void loop_serv();

// Установка угла сервопривода (для API)
void servo_setAngle(uint8_t servoNum, uint16_t angle);

// Получение текущего угла сервопривода
uint16_t servo_getAngle(uint8_t servoNum);

// Установка границ импульсов (для API)
void servo_setLimits(uint8_t servoNum, uint16_t newMin, uint16_t newMax);

// Получение минимального значения импульса
uint16_t servo_getMin(uint8_t servoNum);

// Получение максимального значения импульса
uint16_t servo_getMax(uint8_t servoNum);

#endif