#include "api.h"

#include <Arduino.h>

// Явное объявление WiFi класса для ESP32 Arduino 3.x
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

extern WiFiClass WiFi;

#include "servo.h"
#include "dcmotor.h"
#include "ui.h"
#include "rwifi.h"
#include "apiota.h"

// Веб-сервер на порту 8080
WebServer server(8080);

// Флаг включения логирования
#define API_LOG_ENABLED true

// ===== Вспомогательные функции =====

// Логирование
void api_log(const String& message) {
  if (API_LOG_ENABLED) {
    Serial.println("[API] " + message);
  }
}

// Отправка JSON ответа
void sendJSONResponse(int code, const String& json) {
  api_log("Response [" + String(code) + "]: " + json);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", json);
}

// Отправка HTML страницы
void sendHTMLResponse(const String& html) {
  api_log("Sending HTML page (" + String(html.length()) + " bytes)");
  server.sendHeader("Content-Type", "text/html");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", html);
}

// ===== Обработчики REST API =====

// GET /api/status - статус системы
void handleStatus() {
  api_log("GET /api/status");
  
  JsonDocument doc;

  doc["status"] = "ok";
  doc["ip"] = WiFi.localIP();
  doc["servos_count"] = 4;

  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// GET /api/servo - получить углы всех сервоприводов
void handleGetServos() {
  api_log("GET /api/servo");

  JsonDocument doc;
  JsonArray servos = doc["servos"].to<JsonArray>();

  for (int i = 0; i < 4; i++) {
    JsonObject servo = servos.add<JsonObject>();
    servo["id"] = i;
    servo["angle"] = servo_getAngle(i);
    servo["min"] = servo_getMin(i);
    servo["max"] = servo_getMax(i);
  }

  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// POST /api/servo - установить угол сервопривода
void handleSetServo() {
  api_log("POST /api/servo");
  
  if (!server.hasArg("plain")) {
    api_log("ERROR: No data provided");
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }

  api_log("Request body: " + server.arg("plain"));
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    api_log("ERROR: Invalid JSON - " + String(error.c_str()));
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int id = doc["id"] | -1;
  int angle = doc["angle"] | -1;

  api_log("Servo ID: " + String(id) + ", Angle: " + String(angle));

  if (id < 0 || id > 15) {
    api_log("ERROR: Invalid servo ID: " + String(id));
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }

  if (angle < 0 || angle > 180) {
    api_log("ERROR: Invalid angle: " + String(angle));
    sendJSONResponse(400, "{\"error\":\"Invalid angle (must be 0-180)\"}");
    return;
  }

  servo_setAngle(id, angle);
  api_log("Servo " + String(id) + " set to " + String(angle) + "°");

  JsonDocument response;
  response["success"] = true;
  response["servo"] = id;
  response["angle"] = angle;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/servo/calibrate - калибровка сервопривода
void handleCalibrateServo() {
  api_log("POST /api/servo/calibrate");
  
  if (!server.hasArg("plain")) {
    api_log("ERROR: No data provided");
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }

  api_log("Request body: " + server.arg("plain"));
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    api_log("ERROR: Invalid JSON - " + String(error.c_str()));
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int id = doc["id"] | -1;
  int minVal = doc["min"] | -1;
  int maxVal = doc["max"] | -1;

  api_log("Servo ID: " + String(id) + ", Min: " + String(minVal) + ", Max: " + String(maxVal));

  if (id < 0 || id > 15) {
    api_log("ERROR: Invalid servo ID: " + String(id));
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }

  if (minVal < 0 || minVal > 4095 || maxVal < 0 || maxVal > 4095) {
    api_log("ERROR: Invalid pulse values");
    sendJSONResponse(400, "{\"error\":\"Invalid pulse values (must be 0-4095)\"}");
    return;
  }

  if (minVal >= maxVal) {
    api_log("ERROR: min must be less than max");
    sendJSONResponse(400, "{\"error\":\"min must be less than max\"}");
    return;
  }

  servo_setLimits(id, minVal, maxVal);
  api_log("Servo " + String(id) + " calibrated: min=" + String(minVal) + ", max=" + String(maxVal));

  JsonDocument response;
  response["success"] = true;
  response["servo"] = id;
  response["min"] = minVal;
  response["max"] = maxVal;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// GET /api/motor - получить статус моторов
void handleGetMotors() {
  api_log("GET /api/motor");

  JsonDocument doc;

  doc["motorA"] = motor_getSpeedA();
  doc["motorB"] = motor_getSpeedB();
  doc["motorC"] = motor_getSpeedC();
  doc["motorD"] = motor_getSpeedD();

  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// POST /api/motor - установить скорость моторов
void handleSetMotor() {
  api_log("POST /api/motor");

  if (!server.hasArg("plain")) {
    api_log("ERROR: No data provided");
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }

  api_log("Request body: " + server.arg("plain"));

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    api_log("ERROR: Invalid JSON - " + String(error.c_str()));
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }

  // Motor A
  if (doc["motorA"].is<int>()) {
    int speedA = doc["motorA"];
    api_log("Motor A speed: " + String(speedA));
    if (speedA < -255 || speedA > 255) {
      api_log("ERROR: Invalid motorA speed: " + String(speedA));
      sendJSONResponse(400, "{\"error\":\"Invalid motorA speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedA(speedA);
  }

  // Motor B
  if (doc["motorB"].is<int>()) {
    int speedB = doc["motorB"];
    api_log("Motor B speed: " + String(speedB));
    if (speedB < -255 || speedB > 255) {
      api_log("ERROR: Invalid motorB speed: " + String(speedB));
      sendJSONResponse(400, "{\"error\":\"Invalid motorB speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedB(speedB);
  }

  // Motor C
  if (doc["motorC"].is<int>()) {
    int speedC = doc["motorC"];
    api_log("Motor C speed: " + String(speedC));
    if (speedC < -255 || speedC > 255) {
      api_log("ERROR: Invalid motorC speed: " + String(speedC));
      sendJSONResponse(400, "{\"error\":\"Invalid motorC speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedC(speedC);
  }

  // Motor D
  if (doc["motorD"].is<int>()) {
    int speedD = doc["motorD"];
    api_log("Motor D speed: " + String(speedD));
    if (speedD < -255 || speedD > 255) {
      api_log("ERROR: Invalid motorD speed: " + String(speedD));
      sendJSONResponse(400, "{\"error\":\"Invalid motorD speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedD(speedD);
  }

  JsonDocument response;
  response["success"] = true;
  response["motorA"] = motor_getSpeedA();
  response["motorB"] = motor_getSpeedB();
  response["motorC"] = motor_getSpeedC();
  response["motorD"] = motor_getSpeedD();

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/motor/stop - остановить все моторы
void handleStopMotors() {
  api_log("POST /api/motor/stop");
  
  motor_stopAll();
  api_log("All motors stopped");

  JsonDocument response;
  response["success"] = true;
  response["message"] = "All motors stopped";

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// GET / - главная страница UI
void handleRoot() {
  api_log("GET / (HTML page)");
  
  String html = getUIHTML();
  sendHTMLResponse(html);
  html = String(); // Освобождаем память
}

// OPTIONS запрос для CORS preflight
void handleOptions() {
  api_log("OPTIONS (CORS preflight)");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

// Обработчик неизвестных маршрутов
void handleNotFound() {
  String message = "Not Found: " + String(server.method()) + " " + server.uri();
  api_log(message);

  // Для API эндпоинтов возвращаем JSON ошибку
  if (server.uri().startsWith("/api/")) {
    sendJSONResponse(404, "{\"error\":\"Endpoint not found\"}");
  } else {
    // Для остальных - перенаправляем на главную или возвращаем 404
    server.send(404, "text/plain", "404 Not Found");
  }
}

// ===== Инициализация =====

void api_init() {
  Serial.println("\n=== API Initialization ===");

  // Маршруты UI
  server.on("/", HTTP_GET, handleRoot);

  // OTA маршруты
  apiota_init(server);

  // Маршруты API
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/servo", HTTP_GET, handleGetServos);
  server.on("/api/servo", HTTP_POST, handleSetServo);
  server.on("/api/servo/calibrate", HTTP_POST, handleCalibrateServo);
  server.on("/api/motor", HTTP_GET, handleGetMotors);
  server.on("/api/motor", HTTP_POST, handleSetMotor);
  server.on("/api/motor/stop", HTTP_POST, handleStopMotors);

  // Обработчик неизвестных маршрутов
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started on port 8080");
  Serial.println("================================\n");
}

void api_loop() {
  server.handleClient();
}
