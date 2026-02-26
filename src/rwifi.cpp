#include "rwifi.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>

// ===== Константы =====

#define WIFI_CONNECTION_TIMEOUT_MS 10000
#define WIFI_RETRY_DELAY_MS 500

// ===== Глобальные переменные =====

static bool wifiConnected = false;

// ===== Публичные функции =====

void wifi_init() {
  Serial.println("\n=== WiFi Initialization ===");
  unsigned long start = millis();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && 
         (millis() - startTime) < WIFI_CONNECTION_TIMEOUT_MS) {
    delay(WIFI_RETRY_DELAY_MS);
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
  // Резерв для будущей обработки событий WiFi
  // Например, переподключение или мониторинг клиентов
}

bool wifi_isConnected() {
  return wifiConnected;
}

String wifi_getIP() {
  return WiFi.softAPIP().toString();
}
