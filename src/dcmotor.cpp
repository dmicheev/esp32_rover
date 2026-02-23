#include <Arduino.h>

#include "dcmotor.h"
#include "pins.h"



// Текущие скорости моторов
static int motorSpeedA = 0;
static int motorSpeedB = 0;
static int motorSpeedC = 0;
static int motorSpeedD = 0;

// Функция установки скорости для одного мотора
static void setMotorSpeed(int pinA, int pinB, int speed) {
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

void motor_setSpeedA(int speed) {
  motorSpeedA = speed;
  setMotorSpeed(A_IA, A_IB, speed);
  Serial.print("Motor A speed: ");
  Serial.println(speed);
}

void motor_setSpeedB(int speed) {
  motorSpeedB = speed;
  setMotorSpeed(B_IA, B_IB, speed);
  Serial.print("Motor B speed: ");
  Serial.println(speed);
}

void motor_setSpeedC(int speed) {
  motorSpeedC = speed;
  setMotorSpeed(C_IA, C_IB, speed);
  Serial.print("Motor C speed: ");
  Serial.println(speed);
}

void motor_setSpeedD(int speed) {
  motorSpeedD = speed;
  setMotorSpeed(D_IA, D_IB, speed);
  Serial.print("Motor D speed: ");
  Serial.println(speed);
}

int motor_getSpeedA() {
  return motorSpeedA;
}

int motor_getSpeedB() {
  return motorSpeedB;
}

int motor_getSpeedC() {
  return motorSpeedC;
}

int motor_getSpeedD() {
  return motorSpeedD;
}

void motor_stopAll() {
  motorSpeedA = 0;
  motorSpeedB = 0;
  motorSpeedC = 0;
  motorSpeedD = 0;
  
  analogWrite(A_IA, LOW);
  analogWrite(A_IB, LOW);
  analogWrite(B_IA, LOW);
  analogWrite(B_IB, LOW);
  analogWrite(C_IA, LOW);
  analogWrite(C_IB, LOW);
  analogWrite(D_IA, LOW);
  analogWrite(D_IB, LOW);
  Serial.println("All motors stopped (A, B, C, D)");

  
}