#!/bin/bash
# Скрипт для подготовки OTA файла для ESP32-S3
# ПРИМЕЧАНИЕ: Для OTA через библиотеку Update.h нужен только firmware.bin,
# без bootloader и partition table

BUILD_DIR=".pio/build/esp32-s3-devkitc1-n16r8"
OUTPUT_DIR="ota_builds"

# Создаём папку для OTA файлов
mkdir -p $OUTPUT_DIR

echo "=== ESP32-S3 OTA Binary Preparation ==="

# Проверяем наличие файла прошивки
if [ ! -f "$BUILD_DIR/firmware.bin" ]; then
    echo "ERROR: firmware.bin not found. Run 'pio run' first."
    exit 1
fi

# Копируем только firmware.bin (это единственный файл, который нужен для OTA)
cp "$BUILD_DIR/firmware.bin" "$OUTPUT_DIR/firmware.bin"

if [ $? -eq 0 ]; then
    echo "✓ OTA binary prepared: $OUTPUT_DIR/firmware.bin"
    ls -lh "$OUTPUT_DIR/firmware.bin"
    echo ""
    echo "=== Instructions ==="
    echo "1. Upload firmware.bin via OTA web interface"
    echo "2. Or use curl:"
    echo "   curl -X POST -F \"firmware=@$OUTPUT_DIR/firmware.bin\" http://<robot-ip>:8080/api/ota/upload"
    echo ""
    echo "NOTE: Only firmware.bin is needed for OTA. The Update.h library"
    echo "automatically writes to the correct OTA partition."
else
    echo "✗ Failed to prepare OTA binary"
    exit 1
fi
