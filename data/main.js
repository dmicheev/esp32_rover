const API_BASE = window.location.origin;

// ===== Переключение закладок =====
function switchTab(tabName) {
  document.querySelectorAll('.tab-content').forEach(tab => {
    tab.classList.remove('active');
  });
  document.querySelectorAll('.tab-button').forEach(btn => {
    btn.classList.remove('active');
  });
  document.getElementById('tab-' + tabName).classList.add('active');
  event.target.classList.add('active');

  if (tabName === 'joystick') {
    initJoystick();
  }

  if (tabName === 'camera') {
    loadCameraStatus();
  }
}

function updateServoValue(id, value) {
  document.getElementById('servo' + id + '-value').textContent = value;
}

async function setServo(id) {
  const angle = document.getElementById('servo' + id).value;
  try {
    const response = await fetch(API_BASE + '/api/servo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: id, angle: parseInt(angle) })
    });
    const result = await response.json();
    if (result.success) {
      showMessage('Servo ' + id + ' установлен на ' + angle + '°', 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

async function calibrateServo(id) {
  const minVal = document.getElementById('servo' + id + '-min').value;
  const maxVal = document.getElementById('servo' + id + '-max').value;
  try {
    const response = await fetch(API_BASE + '/api/servo/calibrate', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: id, min: parseInt(minVal), max: parseInt(maxVal) })
    });
    const result = await response.json();
    if (result.success) {
      showMessage('Servo ' + id + ' откалиброван', 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

async function startMotor(motor, speed) {
  try {
    const response = await fetch(API_BASE + '/api/motor', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ['motor' + motor]: speed })
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('motor' + motor + '-speed').textContent = speed;
      updateMotorValue(motor, speed);
    }
  } catch (error) {
    showMessage('Ошибка: ' + error, 'error');
  }
}

async function setMotorSpeed(motor, speed) {
  try {
    const response = await fetch(API_BASE + '/api/motor', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ['motor' + motor]: speed })
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('motor' + motor + '-speed').textContent = speed;
      updateMotorValue(motor, speed);
    }
  } catch (error) {
    showMessage('Ошибка: ' + error, 'error');
  }
}

function updateMotorValue(motor, value) {
  document.getElementById('motor' + motor + '-value').textContent = value;
  document.getElementById('motor' + motor + '-range').value = value;
  document.getElementById('motor' + motor + '-input').value = value;
}

async function setMotorFromSlider(motor) {
  const speed = parseInt(document.getElementById('motor' + motor + '-input').value);
  if (speed < -255 || speed > 255) {
    showMessage('Ошибка: скорость должна быть от -255 до 255', 'error');
    return;
  }
  try {
    const response = await fetch(API_BASE + '/api/motor', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ['motor' + motor]: speed })
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('motor' + motor + '-speed').textContent = speed;
      showMessage('Motor ' + motor + ' установлен на ' + speed, 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

async function setAllMotors() {
  const motorA = parseInt(document.getElementById('all-motorA').value) || 0;
  const motorB = parseInt(document.getElementById('all-motorB').value) || 0;
  const motorC = parseInt(document.getElementById('all-motorC').value) || 0;
  const motorD = parseInt(document.getElementById('all-motorD').value) || 0;

  if ([motorA, motorB, motorC, motorD].some(s => s < -255 || s > 255)) {
    showMessage('Ошибка: скорость должна быть от -255 до 255', 'error');
    return;
  }

  try {
    const response = await fetch(API_BASE + '/api/motor', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        motorA: motorA,
        motorB: motorB,
        motorC: motorC,
        motorD: motorD
      })
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('motorA-speed').textContent = motorA;
      document.getElementById('motorB-speed').textContent = motorB;
      document.getElementById('motorC-speed').textContent = motorC;
      document.getElementById('motorD-speed').textContent = motorD;
      updateMotorValue('A', motorA);
      updateMotorValue('B', motorB);
      updateMotorValue('C', motorC);
      updateMotorValue('D', motorD);
      showMessage('Все моторы установлены: A=' + motorA + ', B=' + motorB + ', C=' + motorC + ', D=' + motorD, 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

function stopMotor(motor) {
}

async function stopAllMotors() {
  try {
    const response = await fetch(API_BASE + '/api/motor/stop', {
      method: 'POST'
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('motorA-speed').textContent = '0';
      document.getElementById('motorB-speed').textContent = '0';
      showMessage('Все моторы остановлены', 'success');
    }
  } catch (error) {
    showMessage('Ошибка: ' + error, 'error');
  }
}

// ===== ЭКСТРЕННАЯ ОСТАНОВКА =====
async function emergencyStopAll() {
  try {
    await fetch(API_BASE + '/api/motor/stop', { method: 'POST' });
    await fetch(API_BASE + '/api/camera/angle', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ pan_angle: 90, tilt_angle: 90 })
    });

    document.getElementById('motorA-speed').textContent = '0';
    document.getElementById('motorB-speed').textContent = '0';
    document.getElementById('motorC-speed').textContent = '0';
    document.getElementById('motorD-speed').textContent = '0';
    updateMotorValue('A', 0);
    updateMotorValue('B', 0);
    updateMotorValue('C', 0);
    updateMotorValue('D', 0);

    document.getElementById('pan-angle-slider').value = 90;
    document.getElementById('pan-angle-input').value = 90;
    document.getElementById('pan-angle-value').textContent = '90';
    document.getElementById('tilt-angle-slider').value = 90;
    document.getElementById('tilt-angle-input').value = 90;
    document.getElementById('tilt-angle-value').textContent = '90';

    showMessage('🚨 ЭКСТРЕННАЯ ОСТАНОВКА выполнена!', 'success');
  } catch (error) {
    showMessage('Ошибка экстренной остановки: ' + error, 'error');
  }
}

async function loadStatus() {
  try {
    const response = await fetch(API_BASE + '/api/status');
    const result = await response.json();
    document.getElementById('robot-ip').textContent = result.ip || 'N/A';
  } catch (error) {
    document.getElementById('robot-ip').textContent = 'Connection error';
  }
}

async function loadServos() {
  try {
    const response = await fetch(API_BASE + '/api/servo');
    const result = await response.json();
    if (result.servos) {
      result.servos.forEach(servo => {
        const id = servo.id;
        document.getElementById('servo' + id).value = servo.angle;
        document.getElementById('servo' + id + '-value').textContent = servo.angle;
        document.getElementById('servo' + id + '-min').value = servo.min;
        document.getElementById('servo' + id + '-max').value = servo.max;
      });
    }
  } catch (error) {
    console.error('Error loading servos:', error);
  }
}

async function loadMotors() {
  try {
    const response = await fetch(API_BASE + '/api/motor');
    const result = await response.json();
    const motorA = result.motorA || 0;
    const motorB = result.motorB || 0;
    const motorC = result.motorC || 0;
    const motorD = result.motorD || 0;

    document.getElementById('motorA-speed').textContent = motorA;
    document.getElementById('motorB-speed').textContent = motorB;
    document.getElementById('motorC-speed').textContent = motorC;
    document.getElementById('motorD-speed').textContent = motorD;
    updateMotorValue('A', motorA);
    updateMotorValue('B', motorB);
    updateMotorValue('C', motorC);
    updateMotorValue('D', motorD);
    document.getElementById('all-motorA').value = motorA;
    document.getElementById('all-motorB').value = motorB;
    document.getElementById('all-motorC').value = motorC;
    document.getElementById('all-motorD').value = motorD;
  } catch (error) {
    console.error('Error loading motors:', error);
  }
}

function showMessage(text, type) {
  const existing = document.querySelector('.message');
  if (existing) existing.remove();

  const msg = document.createElement('div');
  msg.className = 'message ' + type;
  msg.textContent = text;
  document.body.appendChild(msg);

  setTimeout(() => msg.remove(), 3000);
}

window.onload = function () {
  loadStatus();
  loadServos();
  loadMotors();
  setInterval(loadStatus, 5000);
};

// ===== Функции джойстика =====
let joystickInterval = null;
let currentJoystickMode = 'drive';
let lastMotorCommand = null;
let lastSendTime = 0;
const SEND_INTERVAL = 100;

let servoAngles = {
  servo0: 90,
  servo1: 90,
  servo2: 90,
  servo3: 90
};
let lastServoCommand = null;

function initJoystick() {
  const joystick = document.getElementById('joystick');
  const knob = document.getElementById('joystick-knob');
  let isDragging = false;
  const maxRadius = 70;

  knob.addEventListener('mousedown', startDrag);
  document.addEventListener('mousemove', drag);
  document.addEventListener('mouseup', endDrag);
  knob.addEventListener('touchstart', startDrag);
  document.addEventListener('touchmove', drag);
  document.addEventListener('touchend', endDrag);

  function startDrag(e) {
    isDragging = true;
    knob.style.transition = 'none';
  }

  function drag(e) {
    if (!isDragging) return;
    e.preventDefault();

    const rect = joystick.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;

    let clientX, clientY;
    if (e.touches) {
      clientX = e.touches[0].clientX;
      clientY = e.touches[0].clientY;
    } else {
      clientX = e.clientX;
      clientY = e.clientY;
    }

    let dx = clientX - centerX;
    let dy = clientY - centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);

    if (distance > maxRadius) {
      const angle = Math.atan2(dy, dx);
      dx = Math.cos(angle) * maxRadius;
      dy = Math.sin(angle) * maxRadius;
    }

    knob.style.transform = `translate(calc(-50% + ${dx}px), calc(-50% + ${dy}px))`;

    const x = Math.round((dx / maxRadius) * 100);
    const y = Math.round(-(dy / maxRadius) * 100);

    updateJoystickDisplay(x, y);
    updateJoystickControl(x, y);
  }

  function endDrag() {
    isDragging = false;
    knob.style.transition = 'box-shadow 0.2s, transform 0.3s';
    knob.style.transform = 'translate(-50%, -50%)';
    updateJoystickDisplay(0, 0);
    stopMotorsSmooth();
  }
}

function updateJoystickDisplay(x, y) {
  document.getElementById('joy-x').textContent = x;
  document.getElementById('joy-y').textContent = y;
}

function updateJoystickControl(x, y) {
  const now = Date.now();
  if (now - lastSendTime < SEND_INTERVAL) return;

  if (currentJoystickMode === 'drive') {
    const motors = calculateTankDrive(x, y);
    sendMotorCommand(motors);
    updateMotorStatusDisplay(motors);
    updateRoverVisualizer(motors, servoAngles);
    lastMotorCommand = motors;
  } else if (currentJoystickMode === 'servo') {
    const targetAngle = Math.round(90 - x);
    servoAngles.servo0 = constrain(targetAngle, 0, 180);
    servoAngles.servo1 = constrain(targetAngle, 0, 180);
    servoAngles.servo3 = constrain(180 - targetAngle, 0, 180);
    servoAngles.servo2 = constrain(180 - targetAngle, 0, 180);

    const servoCommand = {
      s0: servoAngles.servo0,
      s1: servoAngles.servo1,
      s2: servoAngles.servo2,
      s3: servoAngles.servo3
    };

    if (JSON.stringify(servoCommand) !== JSON.stringify(lastServoCommand)) {
      sendServoCommand(servoCommand);
      lastServoCommand = servoCommand;
    }

    document.getElementById('joy-motorA').textContent = 'S0:' + servoAngles.servo0;
    document.getElementById('joy-motorB').textContent = 'S1:' + servoAngles.servo1;
    document.getElementById('joy-motorC').textContent = 'S2:' + servoAngles.servo2;
    document.getElementById('joy-motorD').textContent = 'S3:' + servoAngles.servo3;
    updateServoVisualizer(servoAngles);

  } else if (currentJoystickMode === 'mixed') {
    const targetAngle = Math.round(90 - x);
    servoAngles.servo0 = constrain(targetAngle, 0, 180);
    servoAngles.servo1 = constrain(targetAngle, 0, 180);
    servoAngles.servo3 = constrain(180 - targetAngle, 0, 180);
    servoAngles.servo2 = constrain(180 - targetAngle, 0, 180);

    const targetSpeed = Math.round(-y * 255 / 100);
    const motors = {
      motorA: targetSpeed,
      motorB: targetSpeed,
      motorC: targetSpeed,
      motorD: targetSpeed
    };

    const servoCommand = {
      s0: servoAngles.servo0,
      s1: servoAngles.servo1,
      s2: servoAngles.servo2,
      s3: servoAngles.servo3
    };

    if (JSON.stringify(servoCommand) !== JSON.stringify(lastServoCommand)) {
      sendServoCommand(servoCommand);
      lastServoCommand = servoCommand;
    }

    sendMotorCommand(motors);
    updateMotorStatusDisplay(motors);
    lastMotorCommand = motors;

    document.getElementById('joy-motorA').textContent = 'S:' + servoAngles.servo0 + ' V:' + targetSpeed;
    document.getElementById('joy-motorB').textContent = 'S:' + servoAngles.servo1 + ' V:' + targetSpeed;
    document.getElementById('joy-motorC').textContent = 'S:' + servoAngles.servo2 + ' V:' + targetSpeed;
    document.getElementById('joy-motorD').textContent = 'S:' + servoAngles.servo3 + ' V:' + targetSpeed;
    updateRoverVisualizer(motors, servoAngles);
  }

  lastSendTime = now;
}

function sendMotorCommand(motors) {
  fetch(API_BASE + '/api/motor', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(motors)
  })
    .then(response => response.json())
    .catch(err => console.error('[Joystick] API error:', err));
}

function sendServoCommand(servoCommand) {
  const promises = [];
  if (servoCommand.s0 !== undefined) {
    promises.push(fetch(API_BASE + '/api/servo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: 0, angle: servoCommand.s0 })
    }).then(r => r.json()));
  }
  if (servoCommand.s1 !== undefined) {
    promises.push(fetch(API_BASE + '/api/servo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: 1, angle: servoCommand.s1 })
    }).then(r => r.json()));
  }
  if (servoCommand.s2 !== undefined) {
    promises.push(fetch(API_BASE + '/api/servo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: 2, angle: servoCommand.s2 })
    }).then(r => r.json()));
  }
  if (servoCommand.s3 !== undefined) {
    promises.push(fetch(API_BASE + '/api/servo', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: 3, angle: servoCommand.s3 })
    }).then(r => r.json()));
  }
  Promise.all(promises).catch(err => console.error('[Joystick Servo] Error:', err));
}

function updateMotorStatusDisplay(motors) {
  document.getElementById('joy-motorA').textContent = motors.motorA;
  document.getElementById('joy-motorB').textContent = motors.motorB;
  document.getElementById('joy-motorC').textContent = motors.motorC;
  document.getElementById('joy-motorD').textContent = motors.motorD;
}

function updateRoverVisualizer(motors, servoAngles) {
  const motorA = motors.motorA || 0;
  const motorB = motors.motorB || 0;
  const motorC = motors.motorC || 0;
  const motorD = motors.motorD || 0;

  updateWheelVisual('wheel-a', motorA, servoAngles.servo1, 'right');
  updateWheelVisual('wheel-b', motorB, servoAngles.servo0, 'left');
  updateWheelVisual('wheel-c', motorC, servoAngles.servo3, 'left');
  updateWheelVisual('wheel-d', motorD, servoAngles.servo2, 'right');
}

function updateWheelVisual(wheelId, speed, servoAngle, side) {
  const wheel = document.getElementById(wheelId);
  const servoArrow = wheel.querySelector('.servo-arrow');
  const speedArrowLine = wheel.querySelector('.speed-arrow-line');
  const speedText = wheel.querySelector('.speed-text');
  const servoAngleText = wheel.querySelector('.servo-angle-text');

  const arrowRotation = servoAngle - 180;
  servoArrow.setAttribute('transform', `rotate(${arrowRotation})`);

  if (servoAngleText) {
    servoAngleText.textContent = servoAngle + '°';
  }

  const arrowLength = Math.abs(speed) * 40 / 255;
  const minArrowLength = speed !== 0 ? 8 : 0;
  const finalLength = Math.max(minArrowLength, arrowLength);
  const direction = speed >= 0 ? 1 : -1;
  const endY = direction * finalLength;

  speedArrowLine.setAttribute('x1', 0);
  speedArrowLine.setAttribute('y1', 0);
  speedArrowLine.setAttribute('x2', 0);
  speedArrowLine.setAttribute('y2', endY);

  if (speedText) {
    speedText.textContent = Math.abs(speed);
  }

  const ellipse = wheel.querySelector('ellipse');
  if (Math.abs(speed) > 10) {
    ellipse.classList.add(side === 'left' ? 'wheel-active-left' : 'wheel-active-right');
  } else {
    ellipse.classList.remove('wheel-active-left', 'wheel-active-right');
  }
}

function updateServoVisualizer(servoAngles) {
  updateRoverVisualizer({
    motorA: 0, motorB: 0, motorC: 0, motorD: 0
  }, servoAngles);
}

function setJoystickMode(mode) {
  currentJoystickMode = mode;
  document.querySelectorAll('.mode-btn').forEach(btn => {
    btn.classList.remove('active');
  });
  document.querySelector(`[data-mode="${mode}"]`).classList.add('active');
  showMessage('Режим: ' + mode, 'success');
}

function stopAllMotorsFromJoystick() {
  stopAllMotors();
  document.getElementById('joy-motorA').textContent = '0';
  document.getElementById('joy-motorB').textContent = '0';
  document.getElementById('joy-motorC').textContent = '0';
  document.getElementById('joy-motorD').textContent = '0';
  document.getElementById('joy-x').textContent = '0';
  document.getElementById('joy-y').textContent = '0';
}

function stopMotorsSmooth() {
  sendMotorCommand({
    motorA: 0,
    motorB: 0,
    motorC: 0,
    motorD: 0
  });
}

function constrain(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

// ===== Функции управления камерой =====

async function loadCameraStatus() {
  try {
    const cameraIp = document.getElementById('camera-ip').value || '192.168.1.111:81';
    const cameraUrl = 'http://' + cameraIp + '/';
    document.getElementById('camera-stream').src = cameraUrl;

    // Обновляем отображение URL в сообщении об ошибке
    const urlDisplay = document.getElementById('camera-url-display');
    if (urlDisplay) {
      urlDisplay.textContent = cameraUrl;
    }

    const response = await fetch(API_BASE + '/api/camera');
    const result = await response.json();

    const panAngle = result.pan_angle || 90;
    const tiltAngle = result.tilt_angle || 90;

    document.getElementById('pan-angle-slider').value = panAngle;
    document.getElementById('pan-angle-input').value = panAngle;
    document.getElementById('pan-angle-value').textContent = panAngle;
    document.getElementById('tilt-angle-slider').value = tiltAngle;
    document.getElementById('tilt-angle-input').value = tiltAngle;
    document.getElementById('tilt-angle-value').textContent = tiltAngle;
  } catch (error) {
    console.error('Error loading camera status:', error);
  }
}

function updateCameraUrl() {
  const cameraIp = document.getElementById('camera-ip').value || '192.168.1.111:81';
  const cameraUrl = 'http://' + cameraIp + '/';
  document.getElementById('camera-stream').src = cameraUrl;
  
  // Обновляем отображение URL в сообщении об ошибке
  const urlDisplay = document.getElementById('camera-url-display');
  if (urlDisplay) {
    urlDisplay.textContent = cameraUrl;
  }
  
  showMessage('URL камеры обновлён: ' + cameraUrl, 'success');
}

function updatePanAngleDisplay(value) {
  document.getElementById('pan-angle-value').textContent = value;
  document.getElementById('pan-angle-input').value = value;
}

function updateTiltAngleDisplay(value) {
  document.getElementById('tilt-angle-value').textContent = value;
  document.getElementById('tilt-angle-input').value = value;
}

async function setCameraAngle() {
  const panAngle = parseInt(document.getElementById('pan-angle-input').value);
  const tiltAngle = parseInt(document.getElementById('tilt-angle-input').value);

  if (panAngle < 0 || panAngle > 180 || tiltAngle < 0 || tiltAngle > 180) {
    showMessage('Ошибка: Угол должен быть от 0 до 180°', 'error');
    return;
  }

  try {
    const response = await fetch(API_BASE + '/api/camera/angle', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ pan_angle: panAngle, tilt_angle: tiltAngle })
    });
    const result = await response.json();
    if (result.success) {
      showMessage('Камера установлена: Pan=' + panAngle + '°, Tilt=' + tiltAngle + '°', 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

async function setCameraAnglePreset(panAngle, tiltAngle) {
  try {
    const response = await fetch(API_BASE + '/api/camera/angle', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ pan_angle: panAngle, tilt_angle: tiltAngle })
    });
    const result = await response.json();
    if (result.success) {
      document.getElementById('pan-angle-slider').value = panAngle;
      document.getElementById('pan-angle-input').value = panAngle;
      document.getElementById('pan-angle-value').textContent = panAngle;
      document.getElementById('tilt-angle-slider').value = tiltAngle;
      document.getElementById('tilt-angle-input').value = tiltAngle;
      document.getElementById('tilt-angle-value').textContent = tiltAngle;
      showMessage('Пресет применён: Pan=' + panAngle + '°, Tilt=' + tiltAngle + '°', 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

async function setCameraPWM() {
  const panPwm = parseInt(document.getElementById('pan-pwm-input').value);
  const tiltPwm = parseInt(document.getElementById('tilt-pwm-input').value);

  if (panPwm < 500 || panPwm > 2370 || tiltPwm < 500 || tiltPwm > 2370) {
    showMessage('Ошибка: ШИМ должен быть от 500 до 2370', 'error');
    return;
  }

  try {
    const response = await fetch(API_BASE + '/api/camera/pwm', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ pan_pwm: panPwm, tilt_pwm: tiltPwm })
    });
    const result = await response.json();
    if (result.success) {
      showMessage('Камера установлена: Pan=' + panPwm + ', Tilt=' + tiltPwm, 'success');
    } else {
      showMessage('Ошибка: ' + result.error, 'error');
    }
  } catch (error) {
    showMessage('Ошибка соединения: ' + error, 'error');
  }
}

