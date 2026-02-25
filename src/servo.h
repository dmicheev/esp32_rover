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

// ===== Функции для точного управления камерой =====

// Установка точного значения PWM для камеры (очень точная регулировка)
void camera_setPWM(uint16_t panPWM, uint16_t tiltPWM);

// Установка только горизонтального положения (Pan)
void camera_setPanPWM(uint16_t pwmValue);

// Установка только вертикального положения (Tilt)
void camera_setTiltPWM(uint16_t pwmValue);

// Плавное движение на заданный шаг (для точной настройки)
void camera_moveStep(int16_t panStep, int16_t tiltStep);

// Центрирование камеры
void camera_center();

// Получение текущих значений PWM камеры
void camera_getPWM(uint16_t* panPWM, uint16_t* tiltPWM);

// Калибровка камеры - установка центрального положения
void camera_calibrateCenter(uint16_t panCenter, uint16_t tiltCenter);

// Калибровка границ движения камеры
void camera_calibrateLimits(uint16_t panMin, uint16_t panMax, uint16_t tiltMin, uint16_t tiltMax);

// Получение конфигурации камеры
void camera_getConfig(uint16_t* panCenter, uint16_t* tiltCenter,
                      uint16_t* panMin, uint16_t* panMax,
                      uint16_t* tiltMin, uint16_t* tiltMax);

#endif