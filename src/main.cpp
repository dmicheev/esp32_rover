#include <Arduino.h>

#include "rwifi.h"
#include "api.h"
#include "servo.h"
#include "dcmotor.h"
#include "ui.h"

void setup() {
  // Инициализация последовательного порта
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("   ESP32-S3 Robot Controller v2.0");
  Serial.println("========================================\n");

  unsigned long totalStart = millis();

  // Инициализация файловой системы LittleFS
  ui_init();

  // Инициализация WiFi
  wifi_init();

  // Инициализация API сервера
  api_init();

  // Инициализация сервоприводов
  setup_serv();

  // Инициализация DC-моторов
  setup_dc();

  Serial.println("\n✓ All systems initialized");
  Serial.print("Total initialization took ");
  Serial.print(millis() - totalStart);
  Serial.println(" ms");
  Serial.println("========================================\n");
}

void loop() {
  // Обработка WiFi событий
  wifi_loop();

  // Обработка HTTP запросов
  api_loop();

  // Обработка команд Serial (для отладки)
  //loop_serv();

  // Обработка DC-моторов
  //loop_dc();

  delay(1);
}

