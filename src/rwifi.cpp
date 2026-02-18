#include "wifi.h"

#include <Arduino.h>

// WiFi для ESP32 Arduino 3.x - подключаем все необходимые заголовки
#include "soc/soc_caps.h"
#if SOC_WIFI_SUPPORTED
#include <WiFi.h>
#include <WiFiAP.h>
#endif

// Конфигурация WiFi
#define WIFI_SSID "netis_2.4G_E86C87"
#define WIFI_PASSWORD "X498g8wsd0"

// Флаг подключения
static bool wifiConnected = false;

void wifi_init() {
  Serial.println("\n=== WiFi Initialization ===");
  
  // Отключаем режим модемы для экономии энергии
  WiFi.setSleep(false);
  
  // Создаём точку доступа
  Serial.print("Creating Access Point: ");
  Serial.println(WIFI_SSID);
  
  bool apResult = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  if (!apResult) {
    Serial.println("ERROR: Failed to create Access Point");
    return;
  }
  
  Serial.println("Access Point created successfully!");
  
  // Ждём присвоения IP адреса
  delay(100);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  Serial.print("AP MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
  
  wifiConnected = true;
  
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
