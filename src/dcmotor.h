#ifndef _DCMOTOR_H
#define _DCMOTOR_H

#include <stdint.h>

// Инициализация DC-моторов
void setup_dc();

// Обработка DC-моторов (должна вызываться в loop)
void loop_dc();

// Установка скорости мотора A (-255...255)
// Положительное значение - вперёд, отрицательное - назад
void motor_setSpeedA(int8_t speed);

// Установка скорости мотора B (-255...255)
void motor_setSpeedB(int8_t speed);

// Установка скорости мотора C (-255...255)
void motor_setSpeedC(int8_t speed);

// Установка скорости мотора D (-255...255)
void motor_setSpeedD(int8_t speed);

// Получение текущей скорости мотора A
int8_t motor_getSpeedA();

// Получение текущей скорости мотора B
int8_t motor_getSpeedB();

// Получение текущей скорости мотора C
int8_t motor_getSpeedC();

// Получение текущей скорости мотора D
int8_t motor_getSpeedD();

// Остановка всех моторов
void motor_stopAll();

#endif