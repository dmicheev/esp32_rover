#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Создаем объект драйвера PCA9685
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


// Пины I2C для ESP32-S3 (попробуйте разные комбинации)
// Стандартные пины для многих ESP32-S3 dev boards:
// GPIO8 (SDA), GPIO9 (SCL) или GPIO21 (SDA), GPIO22 (SCL)
#define I2C_SDA 8
#define I2C_SCL 9

// Альтернативные пины (раскомментируйте при необходимости)
// #define I2C_SDA 8
// #define I2C_SCL 9

// Адрес PCA9685 (по умолчанию 0x40, но может быть 0x41, 0x42 и т.д.)
#define PCA9685_ADDRESS 0x40



// Сканер I2C устройств
void i2cScanner() {
  Serial.println("\n=== Scanning I2C bus ===");
  Serial.printf("SDA: GPIO%d, SCL: GPIO%d\n", I2C_SDA, I2C_SCL);
  
  byte error, address;
  int nDevices = 0;

  if (!Wire.begin(I2C_SDA, I2C_SCL)){
    Serial.println("Wire init ERROR !!!"); 
    return;
  };
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.printf("Device found at address 0x%02X", address);
      
      // Определяем известные устройства
      if (address == 0x40) Serial.println(" -> PCA9685 (default)");
      else if (address == 0x41) Serial.println(" -> PCA9685 (ADDR jumper set)");
      else if (address == 0x42) Serial.println(" -> PCA9685 (ADDR jumpers set)");
      else if (address == 0x43) Serial.println(" -> PCA9685 (ADDR jumpers set)");
      else if (address == 0x44) Serial.println(" -> PCA9685 (ADDR jumpers set)");
      else if (address == 0x60) Serial.println(" -> MCP4725 or similar DAC");
      else if (address == 0x68) Serial.println(" -> MPU6050/MPU9250/GY-521");
      else if (address == 0x76 || address == 0x77) Serial.println(" -> BMP280/BME280");
      else Serial.println();
      
      nDevices++;
    } else if (error == 4) {
      Serial.printf("Unknown error at address 0x%02X\n", address);
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found!");
  } else {
    Serial.printf("Found %d I2C device(s)\n", nDevices);
  }
}

// Проверка работы шины I2C
bool checkI2CBus() {
  Serial.println("Checking I2C bus...");
  
  // Пробуем обратиться к адресу 0x00 (никогда не используется)
  Wire.beginTransmission(0x00);
  byte error = Wire.endTransmission();
  
  if (error == 2) {
    // Адрес NACK - шина работает, но устройство не отвечает
    // Это нормально для адреса 0x00
    Serial.println("I2C bus appears to be working");
    return true;
  } else if (error == 0) {
    Serial.println("WARNING: Unexpected response from address 0x00");
    return true; // Шина работает, но что-то странное
  } else {
    Serial.printf("I2C bus error: %d\n", error);
    return false;
  }
}

// Пробуем альтернативные адреса PCA9685
void tryAlternativeAddresses() {
  Serial.println("\nTrying alternative PCA9685 addresses...");
  byte addresses[] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48};
  const char* addrNames[] = {"Default", "ADDR0", "ADDR0+ADDR1", "ADDR1", 
                            "ADDR2", "ADDR0+ADDR2", "ADDR1+ADDR2", "ADDR0+ADDR1+ADDR2"};
  
  for (int i = 0; i < sizeof(addresses); i++) {
    Wire.beginTransmission(addresses[i]);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.printf("Found PCA9685 at address 0x%02X (%s)\n", addresses[i], 
                   i < 8 ? addrNames[i] : "Unknown config");
    }
  }
}

// Тест связи с PCA9685
void testPCA9685Communication() {
  Serial.println("\nTesting PCA9685 communication...");
  
  // Читаем регистр MODE1 (адрес 0x00)
  Wire.beginTransmission(PCA9685_ADDRESS);
  Wire.write(0x00); // MODE1 register
  Wire.endTransmission();
  
  Wire.requestFrom(PCA9685_ADDRESS, (uint8_t)1);
  if (Wire.available()) {
    byte mode1 = Wire.read();
    Serial.printf("MODE1 register: 0x%02X\n", mode1);
    
    if (mode1 == 0xFF || mode1 == 0x00) {
      Serial.println("WARNING: MODE1 has unexpected value");
    } else {
      Serial.println("PCA9685 communication OK");
    }
  } else {
    Serial.println("ERROR: Cannot read from PCA9685");
  }
}


void init_i2c() {
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32-S3 PCA9685 Servo Controller ===");
  Serial.println("Initializing...");
  
  // Даем время для инициализации Serial
  delay(2000);
  
  // Запускаем сканер I2C для поиска устройств
  i2cScanner();
  
  Serial.println("\nInitializing I2C bus...");
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // Стандартная частота 100 kHz
  
  // Проверяем работу шины I2C
  if (!checkI2CBus()) {
    Serial.println("ERROR: I2C bus not working. Check wiring!");
    while(1) {
      delay(1000);
    }
  }
  
  Serial.println("Initializing PCA9685...");
  
  // Инициализируем PCA9685 с явным указанием адреса
  bool pwmInitSuccess = pwm.begin(PCA9685_ADDRESS);
  
  if (!pwmInitSuccess) {
    Serial.println("ERROR: Failed to initialize PCA9685!");
    Serial.println("Possible causes:");
    Serial.println("1. Wrong I2C address");
    Serial.println("2. Bad wiring connections");
    Serial.println("3. PCA9685 not powered properly");
    Serial.println("4. Wrong I2C pins");
    
    // Пробуем другие адреса
    tryAlternativeAddresses();
    
    Serial.println("\nTrying to initialize with default constructor...");
    pwm = Adafruit_PWMServoDriver(); // Пересоздаем объект
    pwmInitSuccess = pwm.begin();
    
    if (!pwmInitSuccess) {
      Serial.println("Still failing. Please check hardware connections.");
      Serial.println("\nTroubleshooting steps:");
      Serial.println("1. Check 3.3V/5V power to PCA9685");
      Serial.println("2. Check GND connection");
      Serial.println("3. Check SDA/SCL connections");
      Serial.println("4. Check I2C pull-up resistors (4.7k to 3.3V)");
      Serial.println("5. Try different I2C pins");
      
      while(1) {
        Serial.println("System halted. Fix hardware and restart.");
        delay(5000);
      }
    }
  }
  
  Serial.println("PCA9685 initialized successfully!");
  
  // Устанавливаем частоту ШИМ 50 Гц для сервоприводов
  pwm.setPWMFreq(50);
  Serial.println("PWM frequency set to 50Hz");
  
  delay(100);
  
  // Тестируем связь с PCA9685
  testPCA9685Communication();
  
}