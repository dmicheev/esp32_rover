# 📝 Логирование API запросов

## Обзор

В проект добавлено подробное логирование всех HTTP запросов к REST API. Это помогает отлаживать проблемы с подключением и обработкой запросов.

## 🔍 Что логируется

### 1. **Все входящие запросы**

Каждый запрос к API записывается в Serial:

```
[API] GET /api/status
[API] GET /api/servo
[API] POST /api/servo
[API] GET /api/motor
[API] POST /api/motor
[API] POST /api/motor/stop
[API] GET / (HTML page)
[API] OPTIONS (CORS preflight)
```

### 2. **Тела запросов**

Для POST запросов логируется содержимое:

```
[API] Request body: {"id":0,"angle":90}
[API] Servo ID: 0, Angle: 90
[API] Servo 0 set to 90°
```

### 3. **Ответы сервера**

Каждый ответ записывается с кодом состояния:

```
[API] Response [200]: {"success":true,"servo":0,"angle":90}
[API] Response [400]: {"error":"Invalid angle (must be 0-180)"}
[API] Response [404]: {"error":"Endpoint not found"}
```

### 4. **Ошибки**

Все ошибки детально логируются:

```
[API] ERROR: No data provided
[API] ERROR: Invalid JSON - InvalidInput
[API] ERROR: Invalid servo ID: 25
[API] ERROR: Invalid angle: 200
[API] ERROR: Invalid pulse values
```

### 5. **Неизвестные маршруты**

```
[API] Not Found: POST /api/unknown
[API] Not Found: GET /favicon.ico
```

## 🖥️ Просмотр логов

### PlatformIO

```bash
# Открыть Serial Monitor
pio device monitor

# Или с указанием скорости
pio device monitor --baud 115200
```

### Arduino IDE

- Инструменты → Монитор порта
- Скорость: 115200 бод

### Пример вывода

```
========================================
   ESP32-S3 Robot Controller v2.0
========================================

=== LittleFS Initialization ===
LittleFS initialized successfully
index.html found
===============================

=== WiFi Initialization ===
Creating Access Point: RobotAP
Access Point created successfully!
AP IP address: 192.168.4.1
AP MAC address: XX:XX:XX:XX:XX:XX
===========================


=== API Initialization ===
HTTP server started on port 8080
================================

✓ All systems initialized
========================================

[API] GET / (HTML page)
[API] Sending HTML page (15234 bytes)
[API] GET /api/status
[API] Response [200]: {"status":"ok","ip":"192.168.4.1","servos_count":4}
[API] POST /api/servo
[API] Request body: {"id":0,"angle":90}
[API] Servo ID: 0, Angle: 90
[API] Servo 0 set to 90°
[API] Response [200]: {"success":true,"servo":0,"angle":90}
```

## ⚙️ Настройка логирования

### Включение/выключение

В файле `src/api.cpp`:

```cpp
// Флаг включения логирования
#define API_LOG_ENABLED true  // ← Измените на false для отключения
```

### Формат логов

Функция логирования:

```cpp
void api_log(const String& message) {
  if (API_LOG_ENABLED) {
    Serial.println("[API] " + message);
  }
}
```

## 🐛 Отладка проблем

### Проблема: "Все зависает при отправке запроса"

**Решение:**

1. Откройте Serial Monitor
2. Отправьте запрос к API
3. Проверьте логи:
   - Запрос достиг сервера?
   - Есть ли ошибка парсинга JSON?
   - Возвращается ли ответ?

**Пример диагностики:**

```
# Нет логов вообще
→ Проверьте подключение к WiFi
→ Проверьте IP адрес и порт

# Запрос получен, но нет ответа
→ Проверьте на наличие ошибок в логе
→ Возможно проблема в обработке JSON

# Ошибка "Invalid JSON"
→ Проверьте формат отправляемых данных
→ Убедитесь, что Content-Type: application/json
```

### Частые ошибки

#### 1. **404 Not Found**

```
[API] Not Found: GET /api/unknow
```

**Решение:** Проверьте правильность URL

#### 2. **400 Bad Request - No data provided**

```
[API] POST /api/servo
[API] ERROR: No data provided
```

**Решение:** Отправляйте JSON в теле запроса:
```json
{"id":0,"angle":90}
```

#### 3. **400 Bad Request - Invalid JSON**

```
[API] POST /api/servo
[API] ERROR: Invalid JSON - InvalidInput
```

**Решение:** Проверьте формат JSON:
- Используйте двойные кавычки `"`
- Проверьте запятые между полями
- Убедитесь в отсутствии лишних символов

#### 4. **CORS ошибки в браузере**

```
Access to fetch at ... has been blocked by CORS policy
```

**Решение:** Браузер отправляет OPTIONS запрос:
```
[API] OPTIONS (CORS preflight)
```

Сервер автоматически отвечает заголовками:
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

## 📊 Статистика

Логи помогают анализировать использование API:

- Какие эндпоинты используются чаще
- Какие ошибки возникают
- Среднее время обработки запросов

---

**Дата добавления:** Февраль 2026  
**Версия:** 1.0
