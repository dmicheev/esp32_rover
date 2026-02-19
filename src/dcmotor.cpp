#include <Arduino.h>

#include "dcmotor.h"

// Пины для управления моторами
// Мотор A
#define A_IA 14  // Канал A вперёд
#define A_IB 13   // Канал A назад

// Мотор B
#define B_IA 12  // Канал B вперёд
#define B_IB 11  // Канал B назад

// Мотор C
#define C_IA 18  // Канал C вперёд
#define C_IB 17   // Канал C назад

// Мотор D
#define D_IA 16  // Канал D вперёд
#define D_IB 15  // Канал D назад

// Текущие скорости моторов
static int8_t motorSpeedA = 0;
static int8_t motorSpeedB = 0;
static int8_t motorSpeedC = 0;
static int8_t motorSpeedD = 0;

// Функция установки скорости для одного мотора
static void setMotorSpeed(int pinA, int pinB, int8_t speed) {
  speed = constrain(speed, -255, 255);
  
  if (speed > 0) {
    // Движение вперёд
    analogWrite(pinA, speed);
    analogWrite(pinB, 0);
  } else if (speed < 0) {
    // Движение назад
    analogWrite(pinA, 0);
    analogWrite(pinB, abs(speed));
  } else {
    // Остановка
    analogWrite(pinA, 0);
    analogWrite(pinB, 0);
  }
}

void setup_dc() {
  // Настройка пинов мотора A
  pinMode(A_IA, OUTPUT);
  pinMode(A_IB, OUTPUT);

  // Настройка пинов мотора B
  pinMode(B_IA, OUTPUT);
  pinMode(B_IB, OUTPUT);

  // Настройка пинов мотора C
  pinMode(C_IA, OUTPUT);
  pinMode(C_IB, OUTPUT);

  // Настройка пинов мотора D
  pinMode(D_IA, OUTPUT);
  pinMode(D_IB, OUTPUT);

  // Останавливаем моторы при старте
  motor_stopAll();

  Serial.println("DC motors initialized (A, B, C, D)");
}

void loop_dc() {
  // Здесь можно добавить логику автоматического управления
  // Сейчас просто поддерживаем установленные скорости
}

void motor_setSpeedA(int8_t speed) {
  motorSpeedA = speed;
  setMotorSpeed(A_IA, A_IB, speed);
  Serial.print("Motor A speed: ");
  Serial.println(speed);
}

void motor_setSpeedB(int8_t speed) {
  motorSpeedB = speed;
  setMotorSpeed(B_IA, B_IB, speed);
  Serial.print("Motor B speed: ");
  Serial.println(speed);
}

void motor_setSpeedC(int8_t speed) {
  motorSpeedC = speed;
  setMotorSpeed(C_IA, C_IB, speed);
  Serial.print("Motor C speed: ");
  Serial.println(speed);
}

void motor_setSpeedD(int8_t speed) {
  motorSpeedD = speed;
  setMotorSpeed(D_IA, D_IB, speed);
  Serial.print("Motor D speed: ");
  Serial.println(speed);
}

int8_t motor_getSpeedA() {
  return motorSpeedA;
}

int8_t motor_getSpeedB() {
  return motorSpeedB;
}

int8_t motor_getSpeedC() {
  return motorSpeedC;
}

int8_t motor_getSpeedD() {
  return motorSpeedD;
}

void motor_stopAll() {
  motorSpeedA = 0;
  motorSpeedB = 0;
  motorSpeedC = 0;
  motorSpeedD = 0;
  analogWrite(A_IA, 0);
  analogWrite(A_IB, 0);
  analogWrite(B_IA, 0);
  analogWrite(B_IB, 0);
  analogWrite(C_IA, 0);
  analogWrite(C_IB, 0);
  analogWrite(D_IA, 0);
  analogWrite(D_IB, 0);
  Serial.println("All motors stopped (A, B, C, D)");
}