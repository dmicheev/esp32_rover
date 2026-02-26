#include "api.h"
#include "config.h"

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

// ===== Константы =====

#define API_LOG_ENABLED true

#define SERVO_ID_MIN 0
#define SERVO_ID_MAX 15
#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 180
#define PWM_MIN_VALUE 0
#define PWM_MAX_VALUE 4095
#define MOTOR_SPEED_MIN -255
#define MOTOR_SPEED_MAX 255
#define MAX_PULSE_DURATION_MS 5000

// ===== Глобальные объекты =====

WebServer server(HTTP_PORT);

// ===== Вспомогательные функции =====

void api_log(const String& message) {
  if (API_LOG_ENABLED) {
    Serial.println("[API] " + message);
  }
}

void sendJSONResponse(int code, const String& json) {
  api_log("Response [" + String(code) + "]: " + json);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", json);
}

void sendHTMLResponse(const String& html) {
  api_log("Sending HTML page (" + String(html.length()) + " bytes)");
  server.sendHeader("Content-Type", "text/html");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(200, "text/html", html);
}

// Проверка наличия и валидности JSON тела запроса
static bool validateRequestBody(JsonDocument& doc, const String& endpointName) {
  if (!server.hasArg("plain")) {
    api_log("ERROR: No data provided");
    sendJSONResponse(400, "{\"error\":\"No data provided\"}");
    return false;
  }
  
  api_log("Request body: " + server.arg("plain"));
  
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    api_log("ERROR: Invalid JSON - " + String(error.c_str()));
    sendJSONResponse(400, "{\"error\":\"Invalid JSON\"}");
    return false;
  }
  
  return true;
}

// Проверка диапазона значений PWM
static bool isValidPWM(uint16_t value) {
  return value <= PWM_MAX_VALUE;
}

// Проверка диапазона скорости мотора
static bool isValidMotorSpeed(int speed) {
  return speed >= MOTOR_SPEED_MIN && speed <= MOTOR_SPEED_MAX;
}

// Проверка и установка скорости мотора с валидацией
static bool setMotorIfValid(const JsonDocument& doc, const char* motorName, 
                           int speed, void (*setSpeedFunc)(int)) {
  if (doc[motorName].is<int>()) {
    if (!isValidMotorSpeed(speed)) {
      api_log("ERROR: Invalid " + String(motorName) + " speed: " + String(speed));
      sendJSONResponse(400, "{\"error\":\"Invalid " + String(motorName) + 
                 " speed (must be -255 to 255)\"}");
      return false;
    }
    api_log("Motor " + String(motorName) + " speed: " + String(speed));
    setSpeedFunc(speed);
  }
  return true;
}

// ===== Обработчики REST API =====

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

void handleSetServo() {
  api_log("POST /api/servo");
  
  JsonDocument doc;
  if (!validateRequestBody(doc, "servo")) return;
  
  int id = doc["id"] | -1;
  int angle = doc["angle"] | -1;
  
  api_log("Servo ID: " + String(id) + ", Angle: " + String(angle));
  
  if (id < SERVO_ID_MIN || id > SERVO_ID_MAX) {
    api_log("ERROR: Invalid servo ID: " + String(id));
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }
  
  if (angle < SERVO_ANGLE_MIN || angle > SERVO_ANGLE_MAX) {
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

void handleCalibrateServo() {
  api_log("POST /api/servo/calibrate");
  
  JsonDocument doc;
  if (!validateRequestBody(doc, "calibrate")) return;
  
  int id = doc["id"] | -1;
  int minVal = doc["min"] | -1;
  int maxVal = doc["max"] | -1;
  
  api_log("Servo ID: " + String(id) + ", Min: " + String(minVal) + ", Max: " + String(maxVal));
  
  if (id < SERVO_ID_MIN || id > SERVO_ID_MAX) {
    api_log("ERROR: Invalid servo ID: " + String(id));
    sendJSONResponse(400, "{\"error\":\"Invalid servo ID\"}");
    return;
  }
  
  if (!isValidPWM(minVal) || !isValidPWM(maxVal)) {
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

// ===== API для управления камерой =====

void handleGetCamera() {
  api_log("GET /api/camera");
  
  uint16_t panPWM, tiltPWM;
  camera_getPWM(&panPWM, &tiltPWM);
  
  JsonDocument doc;
  doc["pan_pwm"] = panPWM;
  doc["tilt_pwm"] = tiltPWM;
  
  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

void handleSetCameraPWM() {
  api_log("POST /api/camera/pwm");
  
  JsonDocument doc;
  if (!validateRequestBody(doc, "camera pwm")) return;
  
  uint16_t panPWM = doc["pan_pwm"] | 300;
  uint16_t tiltPWM = doc["tilt_pwm"] | 300;

  if (!isValidPWM(panPWM) || !isValidPWM(tiltPWM)) {
    api_log("ERROR: Invalid PWM values");
    sendJSONResponse(400, "{\"error\":\"Invalid PWM values (must be 0-4095)\"}");
    return;
  }

  camera_setPWM(panPWM, tiltPWM);
  api_log("Camera set: PAN=" + String(panPWM) + ", TILT=" + String(tiltPWM));

  JsonDocument response;
  response["success"] = true;
  response["pan_pwm"] = panPWM;
  response["tilt_pwm"] = tiltPWM;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

void handleCameraPulse() {
  api_log("POST /api/camera/pulse");

  JsonDocument doc;
  if (!validateRequestBody(doc, "camera pulse")) return;

  uint16_t panPWM = doc["pan_pwm"] | 300;
  uint16_t tiltPWM = doc["tilt_pwm"] | 300;
  uint16_t durationMs = doc["duration_ms"] | 100;

  durationMs = constrain(durationMs, 0, MAX_PULSE_DURATION_MS);

  if (!isValidPWM(panPWM) || !isValidPWM(tiltPWM)) {
    api_log("ERROR: Invalid PWM values");
    sendJSONResponse(400, "{\"error\":\"Invalid PWM values (must be 0-4095)\"}");
    return;
  }

  camera_pulse(panPWM, tiltPWM, durationMs);
  api_log("Camera pulse: PAN=" + String(panPWM) + ", TILT=" + String(tiltPWM) + 
          ", Duration=" + String(durationMs) + "ms");

  JsonDocument response;
  response["success"] = true;
  response["pan_pwm"] = panPWM;
  response["tilt_pwm"] = tiltPWM;
  response["duration_ms"] = durationMs;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// ===== API для управления моторами =====

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

void handleSetMotor() {
  api_log("POST /api/motor");
  
  JsonDocument doc;
  if (!validateRequestBody(doc, "motor")) return;
  
  if (!setMotorIfValid(doc, "motorA", doc["motorA"], motor_setSpeedA)) return;
  if (!setMotorIfValid(doc, "motorB", doc["motorB"], motor_setSpeedB)) return;
  if (!setMotorIfValid(doc, "motorC", doc["motorC"], motor_setSpeedC)) return;
  if (!setMotorIfValid(doc, "motorD", doc["motorD"], motor_setSpeedD)) return;
  
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

// ===== Маршруты UI =====

void handleRoot() {
  api_log("GET / (HTML page)");
  ui_serveIndex(server);
}

void handleJoystickJS() {
  ui_serveStaticFile(server, "/joystick.js", "application/javascript");
}

void handleStyleCSS() {
  ui_serveStaticFile(server, "/style.css", "text/css");
}

void handleMainJS() {
  ui_serveStaticFile(server, "/main.js", "application/javascript");
}

void handleOptions() {
  api_log("OPTIONS (CORS preflight)");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

void handleNotFound() {
  String message = "Not Found: " + String(server.method()) + " " + server.uri();
  api_log(message);
  
  if (server.uri().startsWith("/api/")) {
    sendJSONResponse(404, "{\"error\":\"Endpoint not found\"}");
  } else {
    server.send(404, "text/plain", "404 Not Found");
  }
}

// ===== Инициализация =====

void api_init() {
  Serial.println("\n=== API Initialization ===");
  
  // Маршруты UI
  server.on("/", HTTP_GET, handleRoot);
  server.on("/joystick.js", HTTP_GET, handleJoystickJS);
  server.on("/style.css", HTTP_GET, handleStyleCSS);
  server.on("/main.js", HTTP_GET, handleMainJS);
  
  // OTA маршруты
  apiota_init(server);
  
  // Маршруты API
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/servo", HTTP_GET, handleGetServos);
  server.on("/api/servo", HTTP_POST, handleSetServo);
  server.on("/api/servo/calibrate", HTTP_POST, handleCalibrateServo);
  
  // Маршруты для управления камерой
  server.on("/api/camera", HTTP_GET, handleGetCamera);
  server.on("/api/camera/pwm", HTTP_POST, handleSetCameraPWM);
  server.on("/api/camera/pulse", HTTP_POST, handleCameraPulse);

  // Маршруты для управления моторами
  server.on("/api/motor", HTTP_GET, handleGetMotors);
  server.on("/api/motor", HTTP_POST, handleSetMotor);
  server.on("/api/motor/stop", HTTP_POST, handleStopMotors);
  
  // Обработчик неизвестных маршрутов
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started on port " + String(HTTP_PORT));
}

void api_loop() {
  server.handleClient();
}
