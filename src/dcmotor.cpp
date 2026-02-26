#include <Arduino.h>

#include "dcmotor.h"
#include "pins.h"

// ===== Константы =====

#define MOTOR_SPEED_MIN -255
#define MOTOR_SPEED_MAX 255
#define MOTOR_COUNT 4

// ===== Структуры данных =====

struct MotorPins {
  int pinA;
  int pinB;
};

struct MotorState {
  int speed;
  MotorPins pins;
};

// ===== Глобальные переменные =====

static MotorState motors[MOTOR_COUNT] = {
  {0, {A_IA, A_IB}},  // Motor A
  {0, {B_IA, B_IB}},  // Motor B
  {0, {C_IA, C_IB}},  // Motor C
  {0, {D_IA, D_IB}}   // Motor D
};

// ===== Вспомогательные функции =====

// Установка скорости для одного мотора
static void setMotorSpeed(MotorState& motor, int speed) {
  speed = constrain(speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  
  if (speed > 0) {
    analogWrite(motor.pins.pinA, speed);
    analogWrite(motor.pins.pinB, 0);
  } else if (speed < 0) {
    analogWrite(motor.pins.pinA, 0);
    analogWrite(motor.pins.pinB, abs(speed));
  } else {
    analogWrite(motor.pins.pinA, 0);
    analogWrite(motor.pins.pinB, 0);
  }
}

// Вывод информации о скорости мотора
static void printMotorSpeed(const char* motorName, int speed) {
  Serial.print("Motor ");
  Serial.print(motorName);
  Serial.print(" speed: ");
  Serial.println(speed);
}

// ===== Публичные функции =====

void setup_dc() {
  const char* motorNames[] = {"A", "B", "C", "D"};
  
  for (int i = 0; i < MOTOR_COUNT; i++) {
    pinMode(motors[i].pins.pinA, OUTPUT);
    pinMode(motors[i].pins.pinB, OUTPUT);
  }
  
  motor_stopAll();
  Serial.println("DC motors initialized (A, B, C, D)");
}

void loop_dc() {
  // Резерв для будущей логики автоматического управления
}

void motor_setSpeedA(int speed) {
  motors[0].speed = speed;
  setMotorSpeed(motors[0], speed);
  printMotorSpeed("A", speed);
}

void motor_setSpeedB(int speed) {
  motors[1].speed = speed;
  setMotorSpeed(motors[1], speed);
  printMotorSpeed("B", speed);
}

void motor_setSpeedC(int speed) {
  motors[2].speed = speed;
  setMotorSpeed(motors[2], speed);
  printMotorSpeed("C", speed);
}

void motor_setSpeedD(int speed) {
  motors[3].speed = speed;
  setMotorSpeed(motors[3], speed);
  printMotorSpeed("D", speed);
}

int motor_getSpeedA() { return motors[0].speed; }
int motor_getSpeedB() { return motors[1].speed; }
int motor_getSpeedC() { return motors[2].speed; }
int motor_getSpeedD() { return motors[3].speed; }

void motor_stopAll() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    motors[i].speed = 0;
    analogWrite(motors[i].pins.pinA, LOW);
    analogWrite(motors[i].pins.pinB, LOW);
  }
  Serial.println("All motors stopped (A, B, C, D)");
}
