#include <Arduino.h>

#include "rwifi.h"
#include "api.h"
#include "servo.h"
#include "dcmotor.h"
#include "ui.h"

// ===== Константы =====

#define SERIAL_BAUD_RATE 115200
#define SERIAL_INIT_DELAY_MS 1000
#define LOOP_DELAY_MS 1

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_INIT_DELAY_MS);

  Serial.println("\n========================================");
  Serial.println("   ESP32-S3 Robot Controller v2.0");
  Serial.println("========================================\n");

  unsigned long totalStart = millis();

  ui_init();
  wifi_init();
  api_init();
  setup_serv();
  setup_dc();

  Serial.println("\n✓ All systems initialized");
  Serial.print("Total initialization took ");
  Serial.print(millis() - totalStart);
  Serial.println(" ms");
  Serial.println("========================================\n");
}

void loop() {
  wifi_loop();
  api_loop();
  delay(LOOP_DELAY_MS);
}
