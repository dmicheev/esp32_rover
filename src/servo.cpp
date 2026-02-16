#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "pins.h"

// Создаем объект драйвера PCA9685
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Настройки для сервоприводов MG90S
// MG90S обычно работает в диапазоне импульсов 500-2400 мкс
// PCA9685 работает с частотой 50 Гц (период 20 мс)
#define SERVOMIN 102   // 102 соответствует 500 мкс (500 / (20000 / 4096))
#define SERVOMAX 512   // 512 соответствует 2500 мкс (2500 / (20000 / 4096))

// Номера сервоприводов на PCA9685
#define SERVO1 2
#define SERVO2 1
#define SERVO3 2
#define SERVO4 3



// Функция для установки угла сервопривода
void setServoAngle(uint8_t servoNum, uint16_t angle) {
  // Ограничиваем угол в диапазоне 0-180 градусов
  angle = constrain(angle, 0, 180);
  
  // Преобразуем угол в импульс
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  
  // Устанавливаем ШИМ сигнал
  if (pwm.setPWM(servoNum, 0, pulse)!= 0){
    Serial.println("ERROR SET PWM !!!");
  };
  
  // Выводим информацию в монитор порта
  Serial.print("Servo ");
  Serial.print(servoNum);
  Serial.print(" set to ");
  Serial.print(angle);
  Serial.print(" degrees, pulse: ");
  Serial.println(pulse);
}

// Альтернативная функция с калибровкой под ваш конкретный сервопривод
void setServoAngleCalibrated(uint8_t servoNum, uint16_t angle, 
                             uint16_t minAngle = 0, uint16_t maxAngle = 180,
                             uint16_t minPulse = SERVOMIN, uint16_t maxPulse = SERVOMAX) {
  angle = constrain(angle, minAngle, maxAngle);
  uint16_t pulse = map(angle, minAngle, maxAngle, minPulse, maxPulse);
  pwm.setPWM(servoNum, 0, pulse);
}

void setup_serv() {
  
  Serial.println("Servo init");
  if (!Wire.begin(I2C_SDA, I2C_SCL)){
    Serial.println("Wire init ERROR !!!"); 
    return;
  };

  // Инициализируем PCA9685
  if (!pwm.begin()){
    Serial.println("PWM init ERROR !!!"); 
    return;
  };
  
  
  // Устанавливаем частоту ШИМ 50 Гц для сервоприводов
  pwm.setPWMFreq(50);
  
  delay(10);
  
  // Устанавливаем сервоприводы в начальное положение
  setServoAngle(SERVO1, 90); // Среднее положение
  //setServoAngle(SERVO2, 90); // Среднее положение
  //setServoAngle(SERVO3, 90); // Среднее положение
  //setServoAngle(SERVO4, 90); // Среднее положение
  
  delay(10);

  Serial.println("Servo init done");
}

void loop_serv() {

  //return;
  // Пример последовательности движений
  
  // Движение серво1 от 0 до 180 градусов
  for (int angle = 80; angle <= 100; angle += 1) {
    setServoAngle(SERVO1, angle);
    delay(100);
  }
  delay(1000);
  return;

  // Движение серво2 от 180 до 0 градусов
  for (int angle = 180; angle >= 0; angle -= 10) {
    setServoAngle(SERVO2, angle);
    delay(100);
  }
  
  // Синхронное движение обоих сервоприводов
  for (int angle = 0; angle <= 180; angle += 5) {
    setServoAngle(SERVO1, angle);
    setServoAngle(SERVO2, 180 - angle); // Противоположное направление

    setServoAngle(SERVO3, angle);
    setServoAngle(SERVO4, 180 - angle); // Противоположное направление
    delay(50);
  }
  
  delay(1000);
}
