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

// ===== Функции для управления камерой (сервы на 360°) =====

// Установка точного значения PWM для камеры (для совместимости с API)
// Для серв на 360°: вращает серву в течение короткого времени, затем останавливает
void camera_setPWM(uint16_t panPWM, uint16_t tiltPWM);

// Подача импульса PWM для камеры на заданное время (для 360° серв)
// panPWM: значение PWM для Pan (отклонение от центра)
// tiltPWM: значение PWM для Tilt (отклонение от центра)
// durationMs: время импульса в миллисекундах
void camera_pulse(uint16_t panPWM, uint16_t tiltPWM, uint16_t durationMs);

// Получение текущих значений PWM камеры (для совместимости с API)
// Возвращает центральное положение (PWM для остановки серв)
void camera_getPWM(uint16_t* panPWM, uint16_t* tiltPWM);

#endif
