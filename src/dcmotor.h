#ifndef _DCMOTOR_H
#define _DCMOTOR_H

#include <stdint.h>

// Инициализация DC-моторов
void setup_dc();

// Обработка DC-моторов (должна вызываться в loop)
void loop_dc();

// Установка скорости мотора A (-255...255)
// Положительное значение - вперёд, отрицательное - назад
void motor_setSpeedA(int speed);

// Установка скорости мотора B (-255...255)
void motor_setSpeedB(int speed);

// Установка скорости мотора C (-255...255)
void motor_setSpeedC(int speed);

// Установка скорости мотора D (-255...255)
void motor_setSpeedD(int speed);

// Получение текущей скорости мотора A
int motor_getSpeedA();

// Получение текущей скорости мотора B
int motor_getSpeedB();

// Получение текущей скорости мотора C
int motor_getSpeedC();

// Получение текущей скорости мотора D
int motor_getSpeedD();

// Остановка всех моторов
void motor_stopAll();

#endif