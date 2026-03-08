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

// ===== Функции для управления камерой (SG92R 180°) =====

// Установка угла камеры (Pan/Tilt) в градусах (0-180)
void camera_setAngle(uint16_t panAngle, uint16_t tiltAngle);

// Получение текущих углов камеры
void camera_getAngle(uint16_t* panAngle, uint16_t* tiltAngle);

// Установка точного значения PWM для камеры (для совместимости с API)
void camera_setPWM(uint16_t panPWM, uint16_t tiltPWM);

// Получение текущих значений PWM камеры
void camera_getPWM(uint16_t* panPWM, uint16_t* tiltPWM);

#endif
