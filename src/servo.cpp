#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "pins.h" // Убедитесь, что там правильные I2C_SDA и I2C_SCL

// Создаем объект драйвера PCA9685
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

// Структура для хранения настроек сервопривода
struct ServoConfig {
  uint16_t minPulse;  // SERVOMIN для этого серво
  uint16_t maxPulse;  // SERVOMAX для этого серво
  uint16_t currentAngle; // Текущий угол для информации
};

// Максимальное количество сервоприводов (PCA9685 имеет 16 каналов)
#define MAX_SERVOS 16

// Массив с настройками для каждого сервопривода
ServoConfig servoConfigs[MAX_SERVOS];

// Номера ваших сервоприводов (можно изменить)
#define SERVO1 0
#define SERVO2 1
#define SERVO3 2
#define SERVO4 3

// Значения по умолчанию для MG90S
#define DEFAULT_SERVOMIN 140
#define DEFAULT_SERVOMAX 480

// Буфер для приема команд из Serial
String inputString = "";

// ===== Объявления внутренних функций =====

void setServoAngle(uint8_t servoNum, uint16_t angle);
void setServoLimits(uint8_t servoNum, uint16_t newMin, uint16_t newMax);

// ===== Публичные функции для API =====

void servo_setAngle(uint8_t servoNum, uint16_t angle) {
  setServoAngle(servoNum, angle);
}

uint16_t servo_getAngle(uint8_t servoNum) {
  if (servoNum >= MAX_SERVOS) return 0;
  return servoConfigs[servoNum].currentAngle;
}

void servo_setLimits(uint8_t servoNum, uint16_t newMin, uint16_t newMax) {
  setServoLimits(servoNum, newMin, newMax);
}

uint16_t servo_getMin(uint8_t servoNum) {
  if (servoNum >= MAX_SERVOS) return DEFAULT_SERVOMIN;
  return servoConfigs[servoNum].minPulse;
}

uint16_t servo_getMax(uint8_t servoNum) {
  if (servoNum >= MAX_SERVOS) return DEFAULT_SERVOMAX;
  return servoConfigs[servoNum].maxPulse;
}

// ===== Внутренние функции =====

void setServoAngle(uint8_t servoNum, uint16_t angle) {
  if (servoNum >= MAX_SERVOS) {
    Serial.println("Error: Servo number out of range");
    return;
  }
  
  // Ограничиваем угол в диапазоне 0-180 градусов
  angle = constrain(angle, 0, 180);
  
  // Получаем настройки для этого сервопривода
  uint16_t minPulse = servoConfigs[servoNum].minPulse;
  uint16_t maxPulse = servoConfigs[servoNum].maxPulse;
  
  // Преобразуем угол в импульс
  uint16_t pulse = map(angle, 0, 180, minPulse, maxPulse);
  
  // Устанавливаем ШИМ сигнал
  pwm.setPWM(servoNum, 0, pulse);
  
  // Сохраняем текущий угол
  servoConfigs[servoNum].currentAngle = angle;
  
  // Выводим информацию в монитор порта
  Serial.print("Servo ");
  Serial.print(servoNum);
  Serial.print(" -> ");
  Serial.print(angle);
  Serial.print("° (pulse: ");
  Serial.print(pulse);
  Serial.print(", min:");
  Serial.print(minPulse);
  Serial.print(", max:");
  Serial.print(maxPulse);
  Serial.println(")");
}

// Функция для изменения границ сервопривода
void setServoLimits(uint8_t servoNum, uint16_t newMin, uint16_t newMax) {
  if (servoNum >= MAX_SERVOS) {
    Serial.println("Error: Servo number out of range");
    return;
  }
  
  // Проверяем корректность значений
  if (newMin >= newMax) {
    Serial.println("Error: MIN must be less than MAX");
    return;
  }
  
  if (newMin < 0 || newMax > 4095) {
    Serial.println("Error: Values must be between 0 and 4095");
    return;
  }
  
  // Сохраняем новые границы
  servoConfigs[servoNum].minPulse = newMin;
  servoConfigs[servoNum].maxPulse = newMax;
  
  Serial.print("Servo ");
  Serial.print(servoNum);
  Serial.print(" limits updated: MIN=");
  Serial.print(newMin);
  Serial.print(", MAX=");
  Serial.println(newMax);
  
  // Если сервопривод уже был в каком-то положении, обновляем его с новыми границами
  if (servoConfigs[servoNum].currentAngle > 0) {
    setServoAngle(servoNum, servoConfigs[servoNum].currentAngle);
  }
}

// Функция для отображения справки
void printHelp() {
  Serial.println("\n========== HELP ==========");
  Serial.println("Commands:");
  Serial.println("  S<servo>,<angle>    - Set servo angle (0-180)");
  Serial.println("  L<servo>,<min>,<max> - Set servo limits (0-4095)");
  Serial.println("  G<servo>            - Get servo info");
  Serial.println("  A                   - Get all servos info");
  Serial.println("  R                   - Reset all servos to 90°");
  Serial.println("  H                   - Show this help");
  Serial.println("Examples:");
  Serial.println("  S0,90  - Set servo 0 to 90°");
  Serial.println("  L0,130,490 - Set limits for servo 0");
  Serial.println("==========================\n");
}

// Функция для получения информации о сервоприводе
void printServoInfo(uint8_t servoNum) {
  if (servoNum >= MAX_SERVOS) {
    Serial.println("Error: Servo number out of range");
    return;
  }
  
  Serial.print("Servo ");
  Serial.print(servoNum);
  Serial.print(": Angle=");
  Serial.print(servoConfigs[servoNum].currentAngle);
  Serial.print("°, MIN=");
  Serial.print(servoConfigs[servoNum].minPulse);
  Serial.print(", MAX=");
  Serial.println(servoConfigs[servoNum].maxPulse);
}

// Функция для получения информации обо всех сервоприводах
void printAllServosInfo() {
  Serial.println("\n=== All Servos Info ===");
  for (int i = 0; i < 4; i++) { // Показываем только первые 4, где есть серво
    printServoInfo(i);
  }
  Serial.println("=======================\n");
}

// Парсинг команд из Serial
void parseCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;
  
  char commandType = cmd.charAt(0);
  String params = cmd.substring(1);
  
  switch (commandType) {
    case 'S': // Установка угла: S<servo>,<angle>
    case 's': {
      int commaIndex = params.indexOf(',');
      if (commaIndex > 0) {
        int servo = params.substring(0, commaIndex).toInt();
        int angle = params.substring(commaIndex + 1).toInt();
        setServoAngle(servo, angle);
      } else {
        Serial.println("Error: Invalid format. Use: S<servo>,<angle>");
      }
      break;
    }
    
    case 'L': // Установка лимитов: L<servo>,<min>,<max>
    case 'l': {
      int firstComma = params.indexOf(',');
      int secondComma = params.indexOf(',', firstComma + 1);
      
      if (firstComma > 0 && secondComma > firstComma) {
        int servo = params.substring(0, firstComma).toInt();
        int minVal = params.substring(firstComma + 1, secondComma).toInt();
        int maxVal = params.substring(secondComma + 1).toInt();
        setServoLimits(servo, minVal, maxVal);
      } else {
        Serial.println("Error: Invalid format. Use: L<servo>,<min>,<max>");
      }
      break;
    }
    
    case 'G': // Информация о серво: G<servo>
    case 'g': {
      int servo = params.toInt();
      printServoInfo(servo);
      break;
    }
    
    case 'A': // Информация обо всех серво
    case 'a': {
      printAllServosInfo();
      break;
    }
    
    case 'R': // Сброс всех в 90°
    case 'r': {
      Serial.println("Resetting all servos to 90°");
      for (int i = 0; i < 4; i++) {
        setServoAngle(i, 90);
        delay(50); // Небольшая задержка между командами
      }
      break;
    }
    
    case 'H': // Помощь
    case 'h':
    case '?': {
      printHelp();
      break;
    }
    
    default: {
      Serial.print("Unknown command: ");
      Serial.println(cmd);
      Serial.println("Type 'H' for help");
      break;
    }
  }
}

void setup_serv() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32-S3 + PCA9685 Servo Controller ===");
  
  // 1. Инициализация I2C с нужными пинами
  if (!Wire.begin(I2C_SDA, I2C_SCL)) {
    Serial.println("Wire init ERROR !!!"); 
    return;
  };
  Serial.println("I2C initialized");

  // 2. Инициализация PCA9685
  if (!pwm.begin()) {
    Serial.println("PWM init ERROR !!!"); 
    return;
  };
  Serial.println("PCA9685 initialized");

  // 3. Устанавливаем частоту 50 Гц
  pwm.setPWMFreq(50);
  Serial.println("PWM frequency set to 50Hz");
  
  // 4. Инициализируем настройки по умолчанию для всех сервоприводов
  for (int i = 0; i < MAX_SERVOS; i++) {
    servoConfigs[i].minPulse = DEFAULT_SERVOMIN;
    servoConfigs[i].maxPulse = DEFAULT_SERVOMAX;
    servoConfigs[i].currentAngle = 0; // Предполагаем, что установим в 90°
  }
  
  // 5. Устанавливаем сервоприводы в среднее положение
  Serial.println("\nInitializing servos to 90°...");
  for (int i = 0; i < 4; i++) {
    setServoAngle(i, 0);
    delay(100);
  }
  
  Serial.println("\n=== System Ready ===");
  printHelp();
}

void loop_serv() {
  // Читаем данные из Serial
  while (Serial.available()) {
    char inChar = (char)Serial.read();
     Serial.print(inChar); // <--- ЭТА СТРОКА ДОБАВЛЯЕТ ОТОБРАЖЕНИЕ ВВОДА
    // Если получили символ конца строки, обрабатываем команду
    if (inChar == '\n' || inChar == '\r') {
      if (inputString.length() > 0) {
        parseCommand(inputString);
        inputString = "";
      }
    } else {
      // Добавляем символ в буфер
      inputString += inChar;
    }
  }
  
  // Можно добавить небольшую задержку, но не обязательно
  delay(10);
}