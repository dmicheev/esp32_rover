#!/bin/bash
# Скрипт для создания OTA-совместимого bin файла для ESP32-S3

BUILD_DIR=".pio/build/esp32-s3-devkitc1-n16r8"
OUTPUT_DIR="ota_builds"

# Создаём папку для OTA файлов
mkdir -p $OUTPUT_DIR

echo "=== ESP32-S3 OTA Binary Creator ==="
echo "Building merged binary..."

# Проверяем наличие файлов
if [ ! -f "$BUILD_DIR/bootloader.bin" ]; then
    echo "ERROR: bootloader.bin not found"
    exit 1
fi

if [ ! -f "$BUILD_DIR/partitions.bin" ]; then
    echo "ERROR: partitions.bin not found"
    exit 1
fi

if [ ! -f "$BUILD_DIR/firmware.bin" ]; then
    echo "ERROR: firmware.bin not found"
    exit 1
fi

# Используем esptool.py напрямую (установлен через pip)
# Создаём полный образ для OTA с bootloader и partition table
# Это необходимо для ESP32-S3

echo "Creating full OTA image with bootloader and partition table..."
esptool.py --chip esp32s3 merge_bin \
    -o "$OUTPUT_DIR/ota_firmware.bin" \
    0x0 "$BUILD_DIR/bootloader.bin" \
    0x8000 "$BUILD_DIR/partitions.bin" \
    0x10000 "$BUILD_DIR/firmware.bin"

if [ $? -eq 0 ]; then
    echo "✓ OTA binary created: $OUTPUT_DIR/ota_firmware.bin"
    ls -lh "$OUTPUT_DIR/ota_firmware.bin"
    echo ""
    echo "=== Instructions ==="
    echo "1. Upload ota_firmware.bin via OTA web interface"
    echo "2. Or use curl:"
    echo "   curl -X POST -F \"firmware=@$OUTPUT_DIR/ota_firmware.bin\" http://<robot-ip>:8080/api/ota/upload"
else
    echo "✗ Failed to create OTA binary"
    exit 1
fi
