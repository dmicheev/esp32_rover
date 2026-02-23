/**
 * Joystick Control Algorithms
 * Алгоритмы управления роботом через джойстик
 */

// ===== Конфигурация =====
const JOYSTICK_CONFIG = {
  deadZone: 10,        // Мёртвая зона джойстика
  maxSpeed: 255,       // Максимальная скорость моторов
  servoStep: 5,        // Шаг изменения сервоприводов
  updateInterval: 50   // Интервал обновления (мс)
};

// ===== Состояние джойстика =====
let joystickState = {
  x: 0,                // Положение по X (-100...100)
  y: 0,                // Положение по Y (-100...100)
  active: false,       // Активен ли джойстик
  mode: 'drive'        // Режим: 'drive', 'servo', 'mixed'
};

// ===== Алгоритмы движения =====

/**
 * Танковое управление (дифференциальное)
 * Конфигурация: 0-B(лев), 1-A(прав), 2-D(прав), 3-C(лев)
 * Левые моторы: B, C | Правые моторы: A, D
 * @param {number} x - Положение джойстика по X
 * @param {number} y - Положение джойстика по Y
 * @returns {Object} Скорости моторов {motorA, motorB, motorC, motorD}
 */
function calculateTankDrive(x, y) {
  // Нормализуем значения
  const throttle = y / 100;  // Газ/тормоз (-1...1)
  const steering = x / 100;   // Поворот (-1...1)

  // Левые моторы (B, C)
  const leftSpeed = (throttle - steering) * JOYSTICK_CONFIG.maxSpeed;
  // Правые моторы (A, D)
  const rightSpeed = (throttle + steering) * JOYSTICK_CONFIG.maxSpeed;

  return {
    motorA: Math.round(constrain(rightSpeed, -255, 255)),  // Правый
    motorB: Math.round(constrain(leftSpeed, -255, 255)),   // Левый
    motorC: Math.round(constrain(leftSpeed, -255, 255)),   // Левый
    motorD: Math.round(constrain(rightSpeed, -255, 255))   // Правый
  };
}

/**
 * Управление всеми колёсами (синхронное)
 * @param {number} x - Поворот
 * @param {number} y - Движение
 * @returns {Object} Скорости моторов
 */
function calculateSyncDrive(x, y) {
  const speed = (y / 100) * JOYSTICK_CONFIG.maxSpeed;
  
  return {
    motorA: Math.round(constrain(speed, -255, 255)),
    motorB: Math.round(constrain(speed, -255, 255)),
    motorC: Math.round(constrain(speed, -255, 255)),
    motorD: Math.round(constrain(speed, -255, 255))
  };
}

/**
 * Управление с поворотом на месте
 * @param {number} x - Поворот
 * @param {number} y - Движение
 * @returns {Object} Скорости моторов
 */
function calculateTurnDrive(x, y) {
  const throttle = y / 100;
  const turn = x / 100;
  
  // Если джойстик отклонён только по X - поворот на месте
  if (Math.abs(throttle) < 0.1 && Math.abs(turn) > 0.1) {
    return {
      motorA: Math.round(-turn * JOYSTICK_CONFIG.maxSpeed),
      motorB: Math.round(turn * JOYSTICK_CONFIG.maxSpeed),
      motorC: Math.round(-turn * JOYSTICK_CONFIG.maxSpeed),
      motorD: Math.round(turn * JOYSTICK_CONFIG.maxSpeed)
    };
  }
  
  // Иначе - обычное движение с поворотом
  return calculateTankDrive(x, y);
}

/**
 * Расчёт углов сервоприводов от джойстика
 * @param {number} x - Положение по X
 * @param {number} y - Положение по Y
 * @param {Object} currentAngles - Текущие углы сервоприводов
 * @returns {Object} Новые углы сервоприводов
 */
function calculateServoControl(x, y, currentAngles) {
  const angles = { ...currentAngles };
  
  // Серво 0 и 1 - горизонтальное движение
  if (Math.abs(x) > JOYSTICK_CONFIG.deadZone) {
    angles.servo0 = constrain(angles.servo0 + (x / 100) * JOYSTICK_CONFIG.servoStep, 0, 180);
    angles.servo1 = constrain(angles.servo1 - (x / 100) * JOYSTICK_CONFIG.servoStep, 0, 180);
  }
  
  // Серво 2 и 3 - вертикальное движение
  if (Math.abs(y) > JOYSTICK_CONFIG.deadZone) {
    angles.servo2 = constrain(angles.servo2 + (y / 100) * JOYSTICK_CONFIG.servoStep, 0, 180);
    angles.servo3 = constrain(angles.servo3 + (y / 100) * JOYSTICK_CONFIG.servoStep, 0, 180);
  }
  
  return angles;
}

/**
 * Смешанное управление (моторы + серво)
 * @param {number} x - Положение по X
 * @param {number} y - Положение по Y
 * @param {boolean} useServo - Использовать ли сервоприводы
 * @returns {Object} Команды для моторов и серво
 */
function calculateMixedControl(x, y, useServo = false) {
  const result = {
    motors: calculateTankDrive(x, y),
    servos: null
  };
  
  if (useServo) {
    result.servos = {
      servo0: 90 + (x / 2),  // Примерное позиционирование
      servo1: 90 - (x / 2)
    };
  }
  
  return result;
}

/**
 * Проверка мёртвой зоны
 * @param {number} value - Значение
 * @returns {number} Нормализованное значение
 */
function applyDeadZone(value) {
  if (Math.abs(value) < JOYSTICK_CONFIG.deadZone) {
    return 0;
  }
  return value;
}

/**
 * Ограничение значения
 * @param {number} value - Значение
 * @param {number} min - Минимум
 * @param {number} max - Максимум
 * @returns {number} Ограниченное значение
 */
function constrain(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

/**
 * Плавное изменение скорости (интерполяция)
 * @param {number} current - Текущая скорость
 * @param {number} target - Целевая скорость
 * @param {number} factor - Коэффициент сглаживания (0.1...1.0)
 * @returns {number} Новая скорость
 */
function smoothTransition(current, target, factor = 0.3) {
  return current + (target - current) * factor;
}

/**
 * Расчёт вектора движения
 * @param {number} x - X координата джойстика
 * @param {number} y - Y координата джойстика
 * @returns {Object} Угол и скорость
 */
function calculateMovementVector(x, y) {
  const magnitude = Math.sqrt(x * x + y * y);
  const angle = Math.atan2(y, x) * (180 / Math.PI);
  
  return {
    angle: angle,
    speed: constrain(magnitude, 0, 100),
    normalized: {
      x: magnitude > 0 ? x / magnitude : 0,
      y: magnitude > 0 ? y / magnitude : 0
    }
  };
}

/**
 * Омни-направление (для роботов с омниколёсами)
 * @param {number} x - X координата
 * @param {number} y - Y координата
 * @param {number} rotation - Вращение
 * @returns {Object} Скорости моторов
 */
function calculateOmniDrive(x, y, rotation = 0) {
  const vector = calculateMovementVector(x, y);
  const cos = Math.cos(vector.angle * Math.PI / 180);
  const sin = Math.sin(vector.angle * Math.PI / 180);
  const speed = vector.speed / 100 * JOYSTICK_CONFIG.maxSpeed;
  
  return {
    motorA: Math.round(speed * (cos - sin) + rotation),
    motorB: Math.round(speed * (cos + sin) - rotation),
    motorC: Math.round(speed * (-cos - sin) - rotation),
    motorD: Math.round(speed * (-cos + sin) + rotation)
  };
}

// ===== Экспорт функций =====
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    JOYSTICK_CONFIG,
    calculateTankDrive,
    calculateSyncDrive,
    calculateTurnDrive,
    calculateServoControl,
    calculateMixedControl,
    applyDeadZone,
    constrain,
    smoothTransition,
    calculateMovementVector,
    calculateOmniDrive
  };
}
