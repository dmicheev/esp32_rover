#include <Arduino.h>

#include "rwifi.h"
#include "api.h"
#include "servo.h"
#include "dcmotor.h"

void setup() {
  // Инициализация последовательного порта
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("   ESP32-S3 Robot Controller v2.0");
  Serial.println("========================================\n");
  
  // Инициализация сервоприводов
  setup_serv();
  
  // Инициализация DC-моторов
  setup_dc();
  
  // Инициализация WiFi
  wifi_init();
  
  // Инициализация API сервера
  api_init();
  
  Serial.println("\n✓ All systems initialized");
  Serial.println("========================================\n");
}

void loop() {
  // Обработка WiFi событий
  wifi_loop();
  
  // Обработка HTTP запросов
  api_loop();
  
  // Обработка команд Serial (для отладки)
  loop_serv();
  
  // Обработка DC-моторов
  loop_dc();
  
  delay(10);
}

