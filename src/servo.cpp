#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "pins.h"
#include "config.h"

// ===== Константы =====

#define MAX_SERVOS 16
#define CAMERA_PAN_CHANNEL  4
#define CAMERA_TILT_CHANNEL 5

#define SERVO_ANGLE_MIN 20
#define SERVO_ANGLE_MAX 160

// SG92R 180°: типичные значения PWM для 50Hz
#define SG92R_PWM_MIN 140
#define SG92R_PWM_MAX 480

// Опциональный макрос для отладочного вывода
// Раскомментируйте для включения подробных сообщений
// #define SERVO_DEBUG

#ifdef SERVO_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// ===== Глобальные объекты =====

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

// ===== Структуры данных =====

struct ServoConfig {
  uint16_t currentAngle;
};

struct CameraConfig {
  uint16_t panAngle;
  uint16_t tiltAngle;
  uint16_t panMin;
  uint16_t panMax;
  uint16_t tiltMin;
  uint16_t tiltMax;
};

// ===== Глобальные переменные =====

ServoConfig servoConfigs[MAX_SERVOS];
int correction[MAX_SERVOS] = SERVO_CORRECTION;

CameraConfig cameraConfig = {
  .panAngle = 90,
  .tiltAngle = 90,
  .panMin = SG92R_PWM_MIN,
  .panMax = SG92R_PWM_MAX,
  .tiltMin = SG92R_PWM_MIN,
  .tiltMax = SG92R_PWM_MAX,
};

String inputString = "";

// ===== Вспомогательные функции =====

// Проверка валидности номера сервопривода
static bool isValidServoNum(uint8_t servoNum) {
  return servoNum < MAX_SERVOS;
}


// ===== Публичные функции API =====

void servo_setAngle(uint8_t servoNum, uint16_t angle) {
  if (!isValidServoNum(servoNum)) {
    DEBUG_PRINTLN("Error: Servo number out of range");
    return;
  }
  
  angle = constrain(angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
  int realAngle = angle + correction[servoNum];
  
  uint16_t pulse = map(realAngle, 0, 180, SG92R_PWM_MIN, SG92R_PWM_MAX);
  
  pwm.setPWM(servoNum, 0, pulse);
  servoConfigs[servoNum].currentAngle = angle;
  
  DEBUG_PRINT("Servo ");
  DEBUG_PRINT(servoNum);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINT(angle);
  DEBUG_PRINTLN("°");
}

uint16_t servo_getAngle(uint8_t servoNum) {
  if (!isValidServoNum(servoNum)) return 0;
  return servoConfigs[servoNum].currentAngle;
}

// ===== Функции управления камерой =====

// Преобразование угла (0-180) в PWM значение
static uint16_t angleToPWM(uint16_t angle, uint16_t minPWM, uint16_t maxPWM) {
  return map(angle, 0, 180, minPWM, maxPWM);
}

void camera_setAngle(uint16_t panAngle, uint16_t tiltAngle) {
  panAngle = constrain(panAngle, 0, 180);
  tiltAngle = constrain(tiltAngle, 0, 180);

  uint16_t panPWM = angleToPWM(panAngle, cameraConfig.panMin, cameraConfig.panMax);
  uint16_t tiltPWM = angleToPWM(tiltAngle, cameraConfig.tiltMin, cameraConfig.tiltMax);

  pwm.setPWM(CAMERA_PAN_CHANNEL, 0, panPWM);
  pwm.setPWM(CAMERA_TILT_CHANNEL, 0, tiltPWM);

  cameraConfig.panAngle = panAngle;
  cameraConfig.tiltAngle = tiltAngle;

  DEBUG_PRINT("Camera set: PAN=");
  DEBUG_PRINT(panAngle);
  DEBUG_PRINT("° (");
  DEBUG_PRINT(panPWM);
  DEBUG_PRINT("), TILT=");
  DEBUG_PRINT(tiltAngle);
  DEBUG_PRINT("° (");
  DEBUG_PRINT(tiltPWM);
  DEBUG_PRINTLN(")");
}

void camera_getAngle(uint16_t* panAngle, uint16_t* tiltAngle) {
  if (panAngle) *panAngle = cameraConfig.panAngle;
  if (tiltAngle) *tiltAngle = cameraConfig.tiltAngle;
}

void camera_setPWM(uint16_t panPWM, uint16_t tiltPWM) {
  panPWM = constrain(panPWM, SG92R_PWM_MIN, SG92R_PWM_MAX);
  tiltPWM = constrain(tiltPWM, SG92R_PWM_MIN, SG92R_PWM_MAX);

  pwm.setPWM(CAMERA_PAN_CHANNEL, 0, panPWM);
  pwm.setPWM(CAMERA_TILT_CHANNEL, 0, tiltPWM);

  DEBUG_PRINT("Camera set PWM: PAN=");
  DEBUG_PRINT(panPWM);
  DEBUG_PRINT(", TILT=");
  DEBUG_PRINTLN(tiltPWM);
}

void camera_getPWM(uint16_t* panPWM, uint16_t* tiltPWM) {
  if (panPWM) {
    *panPWM = angleToPWM(cameraConfig.panAngle, cameraConfig.panMin, cameraConfig.panMax);
  }
  if (tiltPWM) {
    *tiltPWM = angleToPWM(cameraConfig.tiltAngle, cameraConfig.tiltMin, cameraConfig.tiltMax);
  }
}

// ===== Инициализация и цикл =====

void setup_serv() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32-S3 + PCA9685 Servo Controller ===");
  
  if (!Wire.begin(I2C_SDA, I2C_SCL)) {
    Serial.println("Wire init ERROR !!!");
    return;
  }
  Serial.println("I2C initialized");
  
  if (!pwm.begin()) {
    Serial.println("PWM init ERROR !!!");
    return;
  }
  Serial.println("PCA9685 initialized");
  
  pwm.setPWMFreq(50);
  Serial.println("PWM frequency set to 50Hz");
  
  for (int i = 0; i < MAX_SERVOS; i++) {
    servoConfigs[i].currentAngle = 0;
  }
  
  Serial.println("\nInitializing servos to 90°...");
  for (int i = 0; i < 4; i++) {
    servo_setAngle(i, 90);
    delay(100);
  }

  camera_setAngle(90, 90);

  Serial.println("\n=== System Ready ===");
}

void loop_servo() {
  
}
