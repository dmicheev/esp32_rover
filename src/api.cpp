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

// Веб-сервер на порту 8080
WebServer server(HTTP_PORT);

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

// ===== API для управления камерой =====

// GET /api/camera - получить статус камеры
void handleGetCamera() {
  api_log("GET /api/camera");

  uint16_t panPWM, tiltPWM;
  uint16_t panCenter, tiltCenter, panMin, panMax, tiltMin, tiltMax;
  
  camera_getPWM(&panPWM, &tiltPWM);
  camera_getConfig(&panCenter, &tiltCenter, &panMin, &panMax, &tiltMin, &tiltMax);

  JsonDocument doc;
  doc["pan_pwm"] = panPWM;
  doc["tilt_pwm"] = tiltPWM;
  doc["pan_center"] = panCenter;
  doc["tilt_center"] = tiltCenter;
  doc["pan_min"] = panMin;
  doc["pan_max"] = panMax;
  doc["tilt_min"] = tiltMin;
  doc["tilt_max"] = tiltMax;

  String response;
  serializeJson(doc, response);
  sendJSONResponse(200, response);
}

// POST /api/camera/set - установить позицию камеры (PWM значения)
void handleSetCamera() {
  api_log("POST /api/camera/set");
  
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

  // Проверяем наличие полей
  bool hasPan = doc["pan_pwm"].is<uint16_t>();
  bool hasTilt = doc["tilt_pwm"].is<uint16_t>();

  if (!hasPan && !hasTilt) {
    api_log("ERROR: No PWM values provided");
    sendJSONResponse(400, "{\"error\":\"No PWM values provided\"}");
    return;
  }

  uint16_t panPWM = doc["pan_pwm"] | 300;
  uint16_t tiltPWM = doc["tilt_pwm"] | 300;

  // Устанавливаем значения
  if (hasPan && hasTilt) {
    camera_setPWM(panPWM, tiltPWM);
  } else if (hasPan) {
    camera_setPanPWM(panPWM);
  } else if (hasTilt) {
    camera_setTiltPWM(tiltPWM);
  }

  api_log("Camera set: PAN=" + String(panPWM) + ", TILT=" + String(tiltPWM));

  JsonDocument response;
  response["success"] = true;
  response["pan_pwm"] = panPWM;
  response["tilt_pwm"] = tiltPWM;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/camera/move - плавное движение на заданный шаг
void handleMoveCamera() {
  api_log("POST /api/camera/move");
  
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

  int16_t panStep = doc["pan_step"] | 0;
  int16_t tiltStep = doc["tilt_step"] | 0;

  // Ограничиваем шаги для безопасности
  panStep = constrain(panStep, -100, 100);
  tiltStep = constrain(tiltStep, -100, 100);

  camera_moveStep(panStep, tiltStep);

  uint16_t panPWM, tiltPWM;
  camera_getPWM(&panPWM, &tiltPWM);

  api_log("Camera moved: pan_step=" + String(panStep) + ", tilt_step=" + String(tiltStep));

  JsonDocument response;
  response["success"] = true;
  response["pan_step"] = panStep;
  response["tilt_step"] = tiltStep;
  response["pan_pwm"] = panPWM;
  response["tilt_pwm"] = tiltPWM;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/camera/center - центрировать камеру
void handleCenterCamera() {
  api_log("POST /api/camera/center");
  
  camera_center();

  uint16_t panPWM, tiltPWM;
  camera_getPWM(&panPWM, &tiltPWM);

  JsonDocument response;
  response["success"] = true;
  response["message"] = "Camera centered";
  response["pan_pwm"] = panPWM;
  response["tilt_pwm"] = tiltPWM;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/camera/calibrate/center - калибровка центрального положения
void handleCalibrateCameraCenter() {
  api_log("POST /api/camera/calibrate/center");
  
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

  uint16_t panCenter = doc["pan_center"] | 300;
  uint16_t tiltCenter = doc["tilt_center"] | 300;

  if (panCenter < 0 || panCenter > 4095 || tiltCenter < 0 || tiltCenter > 4095) {
    api_log("ERROR: Invalid PWM values");
    sendJSONResponse(400, "{\"error\":\"Invalid PWM values (must be 0-4095)\"}");
    return;
  }

  camera_calibrateCenter(panCenter, tiltCenter);

  JsonDocument response;
  response["success"] = true;
  response["pan_center"] = panCenter;
  response["tilt_center"] = tiltCenter;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  sendJSONResponse(200, jsonResponse);
}

// POST /api/camera/calibrate/limits - калибровка границ движения
void handleCalibrateCameraLimits() {
  api_log("POST /api/camera/calibrate/limits");
  
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

  uint16_t panMin = doc["pan_min"] | 150;
  uint16_t panMax = doc["pan_max"] | 450;
  uint16_t tiltMin = doc["tilt_min"] | 150;
  uint16_t tiltMax = doc["tilt_max"] | 450;

  if (panMin < 0 || panMin > 4095 || panMax < 0 || panMax > 4095 ||
      tiltMin < 0 || tiltMin > 4095 || tiltMax < 0 || tiltMax > 4095) {
    api_log("ERROR: Invalid PWM values");
    sendJSONResponse(400, "{\"error\":\"Invalid PWM values (must be 0-4095)\"}");
    return;
  }

  if (panMin >= panMax || tiltMin >= tiltMax) {
    api_log("ERROR: min must be less than max");
    sendJSONResponse(400, "{\"error\":\"min must be less than max\"}");
    return;
  }

  camera_calibrateLimits(panMin, panMax, tiltMin, tiltMax);

  JsonDocument response;
  response["success"] = true;
  response["pan_min"] = panMin;
  response["pan_max"] = panMax;
  response["tilt_min"] = tiltMin;
  response["tilt_max"] = tiltMax;

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
  ui_serveIndex(server);
}

// GET /joystick.js - скрипт джойстика
void handleJoystickJS() {
  ui_serveStaticFile(server, "/joystick.js", "application/javascript");
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
  server.on("/joystick.js", HTTP_GET, handleJoystickJS);

  // OTA маршруты
  apiota_init(server);

  // Маршруты API
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/servo", HTTP_GET, handleGetServos);
  server.on("/api/servo", HTTP_POST, handleSetServo);
  server.on("/api/servo/calibrate", HTTP_POST, handleCalibrateServo);
  
  // Маршруты для управления камерой
  server.on("/api/camera", HTTP_GET, handleGetCamera);
  server.on("/api/camera/set", HTTP_POST, handleSetCamera);
  server.on("/api/camera/move", HTTP_POST, handleMoveCamera);
  server.on("/api/camera/center", HTTP_POST, handleCenterCamera);
  server.on("/api/camera/calibrate/center", HTTP_POST, handleCalibrateCameraCenter);
  server.on("/api/camera/calibrate/limits", HTTP_POST, handleCalibrateCameraLimits);
  
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
