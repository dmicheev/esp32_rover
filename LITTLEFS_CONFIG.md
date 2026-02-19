# ⚙️ Настройка LittleFS для ESP32-S3

## 📁 Структура папок

```
esp32_S3/
├── data/                    ← Сюда кладём файлы для LittleFS
│   ├── index.html           → Доступен как /index.html
│   ├── css/
│   │   └── style.css        → Доступен как /css/style.css
│   ├── js/
│   │   └── script.js        → Доступен как /js/script.js
│   └── config.json          → Доступен как /config.json
├── src/                     ← Исходный код программы
├── platformio.ini           ← Настройки ProjectIO
└── ...
```

## 🔧 Настройка в platformio.ini

### Базовая настройка

```ini
[env:esp32-s3-devkitc1-n16r8]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
board = esp32-s3-devkitc1-n16r8
framework = arduino

; Тип файловой системы
board_build.filesystem = littlefs
```

### Доступные опции

| Опция | Значение | Описание |
|-------|----------|----------|
| `board_build.filesystem` | `littlefs` | Тип файловой системы (SPIFFS или littlefs) |
| `board_upload.filesystem_size` | `1M`, `2M`, `4M` | Размер раздела LittleFS (опционально) |

### Примеры конфигурации

#### 1. **Минимальная конфигурация**
```ini
[env:esp32-s3-devkitc1-n16r8]
board = esp32-s3-devkitc1-n16r8
board_build.filesystem = littlefs
```

#### 2. **С указанием размера раздела**
```ini
[env:esp32-s3-devkitc1-n16r8]
board = esp32-s3-devkitc1-n16r8
board_build.filesystem = littlefs
board_upload.filesystem_size = 2M
```

#### 3. **Свое расположение папки data**
```ini
[env:esp32-s3-devkitc1-n16r8]
board = esp32-s3-devkitc1-n16r8
board_build.filesystem = littlefs
; PlatformIO всегда использует папку "data" в корне проекта
; Изменить это нельзя, но можно создать символическую ссылку
```

## 📤 Загрузка файлов

### Команды PlatformIO

```bash
# Загрузить файлы из папки data/ в LittleFS
pio run --target uploadfs

# Загрузить только прошивку (без LittleFS)
pio run --target upload

# Загрузить и прошивку, и LittleFS
pio run --target upload
pio run --target uploadfs

# Очистить проект
pio run --target clean
```

### Через VS Code

1. Откройте командную палитру (`Ctrl+Shift+P` / `Cmd+Shift+P`)
2. Выберите:
   - **"PlatformIO: Upload Filesystem Image"** — загрузить LittleFS
   - **"PlatformIO: Upload and Upload Filesystem"** — загрузить всё

## 📊 Что загружается в LittleFS

### Автоматически

PlatformIO сканирует папку `data/` и загружает **все файлы** с сохранением структуры:

```
data/
├── index.html          → /index.html
├── logo.png            → /logo.png
├── css/
│   └── style.css       → /css/style.css
└── js/
    └── app.js          → /js/app.js
```

### Исключения

PlatformIO игнорирует:
- Файлы, начинающиеся с `.` (скрытые)
- Файлы в папке `.git/`
- Файлы, указанные в `.gitignore` (если есть)

## 💻 Доступ к файлам из кода

### Чтение файла

```cpp
#include <LittleFS.h>
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  
  // Инициализация LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Ошибка инициализации LittleFS");
    return;
  }
  
  // Открытие файла на чтение
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Файл не найден");
    return;
  }
  
  // Чтение содержимого
  while (file.available()) {
    Serial.print(file.readString());
  }
  
  file.close();
}
```

### Запись в файл

```cpp
// Открытие файла на запись
File file = LittleFS.open("/config.json", "w");
if (file) {
  file.println("{\"wifi_ssid\":\"MyWiFi\",\"password\":\"secret\"}");
  file.close();
  Serial.println("Файл записан");
}
```

### Проверка наличия файла

```cpp
if (LittleFS.exists("/index.html")) {
  Serial.println("Файл существует");
} else {
  Serial.println("Файл не найден");
}
```

### Удаление файла

```cpp
LittleFS.remove("/old_file.txt");
```

### Создание папки

```cpp
LittleFS.mkdir("/config");
```

## 📏 Размер файлов

### Проверка использования памяти

```cpp
#include <LittleFS.h>

void printLittleFSInfo() {
  FSInfo fs_info;
  LittleFS.info(fs_info);
  
  Serial.println("=== LittleFS Info ===");
  Serial.printf("Total bytes: %u\n", fs_info.totalBytes);
  Serial.printf("Used bytes: %u\n", fs_info.usedBytes);
  Serial.printf("Free bytes: %u\n", fs_info.totalBytes - fs_info.usedBytes);
  Serial.printf("Block size: %u\n", fs_info.blockSize);
  Serial.printf("Page size: %u\n", fs_info.pageSize);
  Serial.printf("Max open files: %u\n", fs_info.maxOpenFiles);
  Serial.printf("Max path length: %u\n", fs_info.maxPathLength);
  Serial.println("=====================");
}
```

### Рекомендации

| Тип файла | Размер | Примечание |
|-----------|--------|------------|
| HTML | 5-20KB | Минимизируйте HTML |
| CSS | 2-10KB | Используйте сжатие |
| JavaScript | 5-15KB | Минифицируйте JS |
| JSON config | 1-5KB | Храните только необходимое |
| Изображения | 10-100KB | Сжимайте, используйте WebP |

**Максимальный размер LittleFS:** зависит от размера flash-памяти (обычно 1-4MB)

## 🛠️ Отладка

### Просмотр файлов в LittleFS

```cpp
#include <LittleFS.h>

void listLittleFS() {
  Serial.println("\n=== LittleFS Contents ===");
  
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    Serial.print("File: ");
    Serial.print(file.name());
    Serial.print("  Size: ");
    Serial.println(file.size());
    
    file = root.openNextFile();
  }
  
  Serial.println("=========================\n");
}
```

### Форматирование LittleFS

```cpp
// Полное форматирование
LittleFS.format();

// Или при инициализации
LittleFS.begin(true);  // true = форматировать при ошибке
```

## ⚠️ Частые проблемы

### 1. **Файлы не загружаются**

**Решение:**
```bash
# Проверьте, что папка data/ существует
ls -la data/

# Очистите проект
pio run --target clean

# Загрузите заново
pio run --target uploadfs
```

### 2. **"No space left on device"**

**Решение:**
- Уменьшите размер файлов
- Увеличьте размер раздела в `platformio.ini`:
  ```ini
  board_upload.filesystem_size = 2M
  ```

### 3. **Файл не найден**

**Причины:**
- Файл не загружен в LittleFS
- Неправильный путь (регистр важен!)
- LittleFS не инициализирован

**Решение:**
```cpp
// Проверьте инициализацию
if (!LittleFS.begin(true)) {
  Serial.println("LittleFS init failed");
  return;
}

// Проверьте наличие файла
if (!LittleFS.exists("/index.html")) {
  Serial.println("File not found!");
  listLittleFS();  // Покажите все файлы
}
```

### 4. **Медленная загрузка файлов**

**Решение:**
- Используйте буферизацию при чтении
- Кэшируйте содержимое в памяти
- Сжимайте файлы (gzip)

## 📚 Примеры использования

### Веб-сервер с файлами из LittleFS

```cpp
#include <WebServer.h>
#include <LittleFS.h>

WebServer server(80);

void setup() {
  LittleFS.begin(true);
  
  // Главная страница
  server.on("/", []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });
  
  // CSS файл
  server.on("/style.css", []() {
    File file = LittleFS.open("/css/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });
  
  // JS файл
  server.on("/script.js", []() {
    File file = LittleFS.open("/js/script.js", "r");
    server.streamFile(file, "application/javascript");
    file.close();
  });
  
  server.begin();
}

void loop() {
  server.handleClient();
}
```

### Чтение конфигурации

```cpp
#include <LittleFS.h>
#include <ArduinoJson.h>

struct Config {
  String wifiSSID;
  String wifiPassword;
  int brightness;
};

Config loadConfig() {
  Config config;
  
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    // Файл не найден, возвращаем значения по умолчанию
    config.wifiSSID = "RobotAP";
    config.wifiPassword = "12345678";
    config.brightness = 100;
    return config;
  }
  
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  
  if (!error) {
    config.wifiSSID = doc["wifi_ssid"] | "RobotAP";
    config.wifiPassword = doc["password"] | "12345678";
    config.brightness = doc["brightness"] | 100;
  }
  
  file.close();
  return config;
}

void saveConfig(Config config) {
  File file = LittleFS.open("/config.json", "w");
  if (file) {
    StaticJsonDocument<256> doc;
    doc["wifi_ssid"] = config.wifiSSID;
    doc["password"] = config.wifiPassword;
    doc["brightness"] = config.brightness;
    serializeJson(doc, file);
    file.close();
  }
}
```

---

## 📖 Дополнительные ресурсы

- [PlatformIO Filesystem](https://docs.platformio.org/en/latest/platforms/espressif32.html#filesystems)
- [ESP32 LittleFS](https://github.com/espressif/arduino-esp32/tree/master/libraries/LittleFS)
- [LittleFS GitHub](https://github.com/littlefs-project/littlefs)

---

**Дата обновления:** Февраль 2026  
**Версия:** 1.0
