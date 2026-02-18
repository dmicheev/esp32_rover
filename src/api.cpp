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

// Веб-сервер на порту 8080
WebServer server(8080);

// ===== Вспомогательные функции =====

// Отправка JSON ответа
void sendJSONResponse(int code, const String& json) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", json);
}

// Отправка HTML страницы
void sendHTMLResponse(const String& html) {
  server.sendHeader("Content-Type", "text/html");
  server.send(200, "text/html", html);
}

// ===== Обработчики REST API =====

// GET /api/status - статус системы
void handleStatus() {
  JsonDocument doc;
  
  doc["status"] = "ok";
  doc["ip"] = WiFi.softAPIP().toString();
  doc["servos_count"] = 4;
  
  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// GET /api/servo - получить углы всех сервоприводов
void handleGetServos() {
  JsonDocument doc;
  JsonArray servos = doc.createNestedArray("servos");
  
  for (int i = 0; i < 4; i++) {
    JsonObject servo = servos.createNestedObject();
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
  if (!server.hasArg("plain")) {
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  int id = doc["id"] | -1;
  int angle = doc["angle"] | -1;
  
  if (id < 0 || id > 15) {
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }
  
  if (angle < 0 || angle > 180) {
    sendJSONResponse(400, "{\"error\":\"Invalid angle (must be 0-180)\"}");
    return;
  }
  
  servo_setAngle(id, angle);
  
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
  if (!server.hasArg("plain")) {
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  int id = doc["id"] | -1;
  int minVal = doc["min"] | -1;
  int maxVal = doc["max"] | -1;
  
  if (id < 0 || id > 15) {
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }
  
  if (minVal < 0 || minVal > 4095 || maxVal < 0 || maxVal > 4095) {
    sendJSONResponse(400, "{\"error\":\"Invalid pulse values (must be 0-4095)\"}");
    return;
  }
  
  if (minVal >= maxVal) {
    sendJSONResponse(400, "{\"error\":\"min must be less than max\"}");
    return;
  }
  
  servo_setLimits(id, minVal, maxVal);
  
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
  JsonDocument doc;
  
  doc["motorA"] = motor_getSpeedA();
  doc["motorB"] = motor_getSpeedB();
  
  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// POST /api/motor - установить скорость моторов
void handleSetMotor() {
  if (!server.hasArg("plain")) {
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Motor A
  if (doc["motorA"].is<int>()) {
    int speedA = doc["motorA"];
    if (speedA < -255 || speedA > 255) {
      sendJSONResponse(400, "{\"error\":\"Invalid motorA speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedA(speedA);
  }
  
  // Motor B
  if (doc["motorB"].is<int>()) {
    int speedB = doc["motorB"];
    if (speedB < -255 || speedB > 255) {
      sendJSONResponse(400, "{\"error\":\"Invalid motorB speed (must be -255 to 255)\"}");
      return;
    }
    motor_setSpeedB(speedB);
  }
  
  JsonDocument response;
  response["success"] = true;
  response["motorA"] = motor_getSpeedA();
  response["motorB"] = motor_getSpeedB();
  
  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/motor/stop - остановить все моторы
void handleStopMotors() {
  motor_stopAll();
  
  JsonDocument response;
  response["success"] = true;
  response["message"] = "All motors stopped";
  
  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// GET / - главная страница UI
void handleRoot() {
  String html = getUIHTML();
  sendHTMLResponse(html);
}

// OPTIONS запрос для CORS preflight
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

// ===== Инициализация =====

void api_init() {
  Serial.println("\n=== API Initialization ===");
  
  // Маршруты UI
  server.on("/", HTTP_GET, handleRoot);
  
  // Маршруты API
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/servo", HTTP_GET, handleGetServos);
  server.on("/api/servo", HTTP_POST, handleSetServo);
  server.on("/api/servo/calibrate", HTTP_POST, handleCalibrateServo);
  server.on("/api/motor", HTTP_GET, handleGetMotors);
  server.on("/api/motor", HTTP_POST, handleSetMotor);
  server.on("/api/motor/stop", HTTP_POST, handleStopMotors);
  
  // CORS preflight
  server.onNotFound(handleOptions);
  
  server.begin();
  Serial.println("HTTP server started on port 8080");
  Serial.println("================================\n");
}

void api_loop() {
  server.handleClient();
}
