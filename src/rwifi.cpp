#include "rwifi.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>


// Флаг подключения
static bool wifiConnected = false;

void wifi_init() {
  Serial.println("\n=== WiFi Initialization ===");
  unsigned long start = millis();

  // Отключаем режим модемы для экономии энергии
  //WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Устанавливаем таймаут 10 секунд вместо бесконечного ожидания
  unsigned long timeout = 10000;  // 10 секунд
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP MAC address: ");
    Serial.println(WiFi.macAddress());
    wifiConnected = true;
  } else {
    Serial.println("\nWiFi connection timeout!");
    Serial.println("Robot will work in AP mode...");
  }

  Serial.print("WiFi initialization took ");
  Serial.print(millis() - start);
  Serial.println(" ms");
  Serial.println("===========================\n");
}

void wifi_loop() {
  // Здесь можно добавить обработку событий WiFi при необходимости
  // Например, переподключение или мониторинг клиентов
}

bool wifi_isConnected() {
  return wifiConnected;
}

String wifi_getIP() {
  return WiFi.softAPIP().toString();
}
