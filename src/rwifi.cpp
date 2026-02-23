#include "rwifi.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>


// Флаг подключения
static bool wifiConnected = false;

void wifi_init() {
  Serial.println("\n=== WiFi Initialization ===");

  // Отключаем режим модемы для экономии энергии
  //WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Запускаем процесс подключения [citation:1][citation:2][citation:4]

  // Цикл ожидания подключения
  while (WiFi.status() != WL_CONNECTED) { // Проверяем статус [citation:1][citation:2][citation:4]
    delay(500); // Ждем полсекунды [citation:1][citation:2][citation:4]
    Serial.print("."); // Печатаем точки, чтобы видеть процесс [citation:1][citation:2][citation:4]
  }

  Serial.println("Access Point created successfully!");

  // Ждём присвоения IP адреса
  delay(100);


  Serial.print("AP IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("AP MAC address: ");
  Serial.println(WiFi.macAddress());

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
