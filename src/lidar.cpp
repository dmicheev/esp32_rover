#include "pins.h"
#include "config.h"

#include "Adafruit_VL53L0X.h"
#include <Wire.h>

// Адрес мультиплексора TCA9548A (обычно 0x70)
#define TCA_ADDR 0x70

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// --- Функция переключения канала мультиплексора ---
void tcaSelect(uint8_t channel) {
  if (channel > 7) return; // У мультиплексора только 8 каналов (0-7)
  
  Wire1.beginTransmission(TCA_ADDR);
  // Мы отправляем 1 байт, где каждый бит соответствует каналу
  // 1 << channel означает: для канала 0 отправим 0b00000001, для канала 1 - 0b00000010 и т.д.
  Wire1.write(1 << channel); 
  Wire1.endTransmission();
}

void lidar_init() {


  // Инициализация I2C (ESP32)
  Wire1.begin(I2C2_SDA, I2C2_SCL); 

  Serial.println("Запуск системы с мультиплексором...");

  // 1. ВАЖНО: Сначала выбираем канал, на котором висит датчик (например, канал 0)
  tcaSelect(0);

  // 2. Теперь инициализируем датчик, как будто он подключен напрямую
  if (!lox.begin(VL53L0X_I2C_ADDR, false, &Wire1)) {
    Serial.println(F("Ошибка: VL53L0X не найден на канале 0!"));
    return;
  }
  
  Serial.println(F("Датчик на канале 0 найден!"));
}

void lidar_loop() {
  static unsigned long lastLidarRead = 0;
  const unsigned long lidarInterval = 100; // Интервал между измерениями в мс
  
  unsigned long currentMillis = millis();
  if (currentMillis - lastLidarRead >= lidarInterval) {
    lastLidarRead = currentMillis;
    
    // 1. Выбираем канал датчика
    tcaSelect(0);

    // 2. Делаем замер
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {
      Serial.print("Канал 0, Расстояние (мм): ");
      Serial.println(measure.RangeMilliMeter);
    } else {
      Serial.println("Канал 0: Объект вне зоны видимости");
    }
  }
}