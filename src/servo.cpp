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
#define PWM_MIN_VALUE 0
#define PWM_MAX_VALUE 4095

#define CAMERA_CENTER_PWM 300
#define CAMERA_MIN_PWM 200
#define CAMERA_MAX_PWM 400

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
  uint16_t minPulse;
  uint16_t maxPulse;
  uint16_t currentAngle;
};

struct CameraConfig {
  uint16_t panCenter;
  uint16_t tiltCenter;
  uint16_t panMin;
  uint16_t panMax;
  uint16_t tiltMin;
  uint16_t tiltMax;
};

// ===== Глобальные переменные =====

ServoConfig servoConfigs[MAX_SERVOS];
int correction[MAX_SERVOS] = SERVO_CORRECTION;
CameraConfig cameraConfig = {
  .panCenter = CAMERA_CENTER_PWM,
  .tiltCenter = CAMERA_CENTER_PWM,
  .panMin = CAMERA_MIN_PWM,
  .panMax = CAMERA_MAX_PWM,
  .tiltMin = CAMERA_MIN_PWM,
  .tiltMax = CAMERA_MAX_PWM,
};

String inputString = "";

// ===== Вспомогательные функции =====

// Проверка валидности номера сервопривода
static bool isValidServoNum(uint8_t servoNum) {
  return servoNum < MAX_SERVOS;
}

// Ограничение PWM значений камеры
static void constrainCameraPWM(uint16_t& panPWM, uint16_t& tiltPWM) {
  panPWM = constrain(panPWM, cameraConfig.panMin, cameraConfig.panMax);
  tiltPWM = constrain(tiltPWM, cameraConfig.tiltMin, cameraConfig.tiltMax);
}

// Вывод отладочной информации о сервоприводе
static void printServoDebug(uint8_t servoNum, uint16_t angle, uint16_t pulse,
                            uint16_t minPulse, uint16_t maxPulse) {
  DEBUG_PRINT("Servo ");
  DEBUG_PRINT(servoNum);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINT(angle);
  DEBUG_PRINT("° (pulse: ");
  DEBUG_PRINT(pulse);
  DEBUG_PRINT(", min:");
  DEBUG_PRINT(minPulse);
  DEBUG_PRINT(", max:");
  DEBUG_PRINT(maxPulse);
  DEBUG_PRINTLN(")");
}

// ===== Публичные функции API =====

void servo_setAngle(uint8_t servoNum, uint16_t angle) {
  if (!isValidServoNum(servoNum)) {
    DEBUG_PRINTLN("Error: Servo number out of range");
    return;
  }
  
  angle = constrain(angle, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
  int realAngle = angle + correction[servoNum];
  
  uint16_t minPulse = servoConfigs[servoNum].minPulse;
  uint16_t maxPulse = servoConfigs[servoNum].maxPulse;
  uint16_t pulse = map(realAngle, 0, 180, minPulse, maxPulse);
  
  pwm.setPWM(servoNum, 0, pulse);
  servoConfigs[servoNum].currentAngle = angle;
  
  printServoDebug(servoNum, angle, pulse, minPulse, maxPulse);
}

uint16_t servo_getAngle(uint8_t servoNum) {
  if (!isValidServoNum(servoNum)) return 0;
  return servoConfigs[servoNum].currentAngle;
}

void servo_setLimits(uint8_t servoNum, uint16_t newMin, uint16_t newMax) {
  if (!isValidServoNum(servoNum)) {
    DEBUG_PRINTLN("Error: Servo number out of range");
    return;
  }
  
  if (newMin >= newMax) {
    DEBUG_PRINTLN("Error: MIN must be less than MAX");
    return;
  }
  
  if (newMin > PWM_MAX_VALUE || newMax > PWM_MAX_VALUE) {
    DEBUG_PRINTLN("Error: Values must be between 0 and 4095");
    return;
  }
  
  servoConfigs[servoNum].minPulse = newMin;
  servoConfigs[servoNum].maxPulse = newMax;
  
  DEBUG_PRINT("Servo ");
  DEBUG_PRINT(servoNum);
  DEBUG_PRINT(" limits updated: MIN=");
  DEBUG_PRINT(newMin);
  DEBUG_PRINT(", MAX=");
  DEBUG_PRINTLN(newMax);
  
  if (servoConfigs[servoNum].currentAngle > 0) {
    servo_setAngle(servoNum, servoConfigs[servoNum].currentAngle);
  }
}

uint16_t servo_getMin(uint8_t servoNum) {
  if (!isValidServoNum(servoNum)) return DEFAULT_SERVOMIN;
  return servoConfigs[servoNum].minPulse;
}

uint16_t servo_getMax(uint8_t servoNum) {
  if (!isValidServoNum(servoNum)) return DEFAULT_SERVOMAX;
  return servoConfigs[servoNum].maxPulse;
}

// ===== Функции управления камерой =====

void camera_setPWM(uint16_t panPWM, uint16_t tiltPWM) {
  constrainCameraPWM(panPWM, tiltPWM);
  
  pwm.setPWM(CAMERA_PAN_CHANNEL, 0, panPWM);
  pwm.setPWM(CAMERA_TILT_CHANNEL, 0, tiltPWM);
  
  DEBUG_PRINT("Camera set: PAN=");
  DEBUG_PRINT(panPWM);
  DEBUG_PRINT(", TILT=");
  DEBUG_PRINTLN(tiltPWM);
}

void camera_pulse(uint16_t panPWM, uint16_t tiltPWM, uint16_t durationMs) {
  constrainCameraPWM(panPWM, tiltPWM);
  
  pwm.setPWM(CAMERA_PAN_CHANNEL, 0, panPWM);
  pwm.setPWM(CAMERA_TILT_CHANNEL, 0, tiltPWM);
  
  DEBUG_PRINT("Camera pulse: PAN=");
  DEBUG_PRINT(panPWM);
  DEBUG_PRINT(", TILT=");
  DEBUG_PRINT(tiltPWM);
  DEBUG_PRINT(", Duration=");
  DEBUG_PRINT(durationMs);
  DEBUG_PRINTLN("ms");
  
  delay(durationMs);
  
  DEBUG_PRINTLN("Camera stopped (center position)");
}

void camera_getPWM(uint16_t* panPWM, uint16_t* tiltPWM) {
  if (panPWM) *panPWM = cameraConfig.panCenter;
  if (tiltPWM) *tiltPWM = cameraConfig.tiltCenter;
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
    servoConfigs[i].minPulse = DEFAULT_SERVOMIN;
    servoConfigs[i].maxPulse = DEFAULT_SERVOMAX;
    servoConfigs[i].currentAngle = 0;
  }
  
  Serial.println("\nInitializing servos to 90°...");
  for (int i = 0; i < 4; i++) {
    servo_setAngle(i, 90);
    delay(100);
  }
  
  camera_setPWM(cameraConfig.panCenter, cameraConfig.tiltCenter);
  
  Serial.println("\n=== System Ready ===");
}

void loop_servo() {
  
}
