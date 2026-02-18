#include "ui.h"

// HTML шаблон интерфейса управления роботом
String getUIHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Robot Control Panel</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      min-height: 100vh;
      padding: 20px;
      color: #fff;
    }
    
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    
    h1 {
      text-align: center;
      margin-bottom: 30px;
      color: #00d9ff;
      text-shadow: 0 0 10px rgba(0, 217, 255, 0.5);
    }
    
    .status-bar {
      background: rgba(255, 255, 255, 0.1);
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 30px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 10px;
    }
    
    .status-item {
      display: flex;
      align-items: center;
      gap: 10px;
    }
    
    .status-led {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: #00ff88;
      box-shadow: 0 0 10px #00ff88;
    }
    
    .panel {
      background: rgba(255, 255, 255, 0.05);
      border-radius: 15px;
      padding: 25px;
      margin-bottom: 25px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    
    .panel h2 {
      margin-bottom: 20px;
      color: #00d9ff;
      border-bottom: 2px solid #00d9ff;
      padding-bottom: 10px;
    }
    
    .servo-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
    }
    
    .servo-card {
      background: rgba(255, 255, 255, 0.08);
      border-radius: 10px;
      padding: 20px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    
    .servo-card h3 {
      margin-bottom: 15px;
      color: #ffcc00;
    }
    
    .servo-control {
      margin-bottom: 15px;
    }
    
    .servo-control label {
      display: block;
      margin-bottom: 8px;
      font-size: 14px;
      color: #aaa;
    }
    
    input[type="range"] {
      width: 100%;
      height: 8px;
      border-radius: 4px;
      background: #333;
      outline: none;
      -webkit-appearance: none;
    }
    
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #00d9ff;
      cursor: pointer;
      box-shadow: 0 0 10px rgba(0, 217, 255, 0.5);
    }
    
    .value-display {
      text-align: center;
      font-size: 24px;
      font-weight: bold;
      color: #00d9ff;
      margin: 10px 0;
    }
    
    .motor-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
    }
    
    .motor-card {
      background: rgba(255, 255, 255, 0.08);
      border-radius: 10px;
      padding: 20px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    
    .motor-card h3 {
      margin-bottom: 15px;
      color: #ff6b6b;
    }
    
    .motor-controls {
      display: flex;
      gap: 10px;
      align-items: center;
      margin-bottom: 15px;
    }
    
    .motor-btn {
      flex: 1;
      padding: 15px;
      border: none;
      border-radius: 8px;
      font-size: 24px;
      cursor: pointer;
      transition: all 0.2s;
    }
    
    .btn-reverse {
      background: #ff6b6b;
      color: white;
    }
    
    .btn-reverse:hover {
      background: #ff5252;
    }
    
    .btn-stop {
      background: #888;
      color: white;
      flex: 0.5;
    }
    
    .btn-stop:hover {
      background: #666;
    }
    
    .btn-forward {
      background: #00ff88;
      color: #1a1a2e;
    }
    
    .btn-forward:hover {
      background: #00ff6a;
    }
    
    .motor-speed {
      text-align: center;
      font-size: 20px;
      font-weight: bold;
      color: #00ff88;
    }
    
    .calibrate-section {
      margin-top: 20px;
      padding-top: 15px;
      border-top: 1px solid rgba(255, 255, 255, 0.1);
    }
    
    .calibrate-section h4 {
      margin-bottom: 10px;
      color: #aaa;
      font-size: 14px;
    }
    
    .calibrate-inputs {
      display: flex;
      gap: 10px;
      margin-bottom: 10px;
    }
    
    .calibrate-inputs input {
      flex: 1;
      padding: 8px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      border-radius: 5px;
      background: rgba(255, 255, 255, 0.1);
      color: white;
      font-size: 14px;
    }
    
    .btn-calibrate {
      width: 100%;
      padding: 10px;
      background: #ffcc00;
      color: #1a1a2e;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-weight: bold;
      transition: background 0.2s;
    }
    
    .btn-calibrate:hover {
      background: #ffdb4d;
    }
    
    .btn-apply {
      width: 100%;
      padding: 12px;
      background: #00d9ff;
      color: #1a1a2e;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-weight: bold;
      font-size: 16px;
      margin-top: 10px;
      transition: all 0.2s;
    }
    
    .btn-apply:hover {
      background: #00e5ff;
      box-shadow: 0 0 15px rgba(0, 217, 255, 0.5);
    }
    
    .btn-stop-all {
      background: #ff4444;
      color: white;
      border: none;
      padding: 15px 30px;
      border-radius: 8px;
      font-size: 16px;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.2s;
    }
    
    .btn-stop-all:hover {
      background: #ff0000;
      box-shadow: 0 0 20px rgba(255, 0, 0, 0.5);
    }
    
    .motor-actions {
      display: flex;
      justify-content: center;
      margin-top: 20px;
    }
    
    .message {
      position: fixed;
      top: 20px;
      right: 20px;
      padding: 15px 25px;
      border-radius: 8px;
      color: white;
      font-weight: bold;
      z-index: 1000;
      animation: slideIn 0.3s ease;
    }
    
    .message.success {
      background: #00ff88;
      color: #1a1a2e;
    }
    
    .message.error {
      background: #ff4444;
    }
    
    @keyframes slideIn {
      from {
        transform: translateX(100%);
        opacity: 0;
      }
      to {
        transform: translateX(0);
        opacity: 1;
      }
    }
    
    @media (max-width: 600px) {
      .status-bar {
        flex-direction: column;
        align-items: flex-start;
      }
      
      .motor-controls {
        flex-direction: column;
      }
      
      .btn-stop {
        flex: 1;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🤖 Robot Control Panel</h1>
    
    <div class="status-bar">
      <div class="status-item">
        <div class="status-led"></div>
        <span>System Online</span>
      </div>
      <div class="status-item">
        <span>IP: <strong id="robot-ip">Loading...</strong></span>
      </div>
    </div>
    
    <!-- Сервоприводы -->
    <div class="panel">
      <h2>🔧 Сервоприводы</h2>
      <div class="servo-grid">
        <div class="servo-card">
          <h3>Servo 0</h3>
          <div class="servo-control">
            <label>Угол: <span id="servo0-value">90</span>°</label>
            <input type="range" id="servo0" min="0" max="180" value="90" oninput="updateServoValue(0, this.value)">
          </div>
          <button class="btn-apply" onclick="setServo(0)">Применить</button>
          <div class="calibrate-section">
            <h4>Калибровка</h4>
            <div class="calibrate-inputs">
              <input type="number" id="servo0-min" placeholder="Min" value="140">
              <input type="number" id="servo0-max" placeholder="Max" value="480">
            </div>
            <button class="btn-calibrate" onclick="calibrateServo(0)">Калибровать</button>
          </div>
        </div>
        
        <div class="servo-card">
          <h3>Servo 1</h3>
          <div class="servo-control">
            <label>Угол: <span id="servo1-value">90</span>°</label>
            <input type="range" id="servo1" min="0" max="180" value="90" oninput="updateServoValue(1, this.value)">
          </div>
          <button class="btn-apply" onclick="setServo(1)">Применить</button>
          <div class="calibrate-section">
            <h4>Калибровка</h4>
            <div class="calibrate-inputs">
              <input type="number" id="servo1-min" placeholder="Min" value="140">
              <input type="number" id="servo1-max" placeholder="Max" value="480">
            </div>
            <button class="btn-calibrate" onclick="calibrateServo(1)">Калибровать</button>
          </div>
        </div>
        
        <div class="servo-card">
          <h3>Servo 2</h3>
          <div class="servo-control">
            <label>Угол: <span id="servo2-value">90</span>°</label>
            <input type="range" id="servo2" min="0" max="180" value="90" oninput="updateServoValue(2, this.value)">
          </div>
          <button class="btn-apply" onclick="setServo(2)">Применить</button>
          <div class="calibrate-section">
            <h4>Калибровка</h4>
            <div class="calibrate-inputs">
              <input type="number" id="servo2-min" placeholder="Min" value="140">
              <input type="number" id="servo2-max" placeholder="Max" value="480">
            </div>
            <button class="btn-calibrate" onclick="calibrateServo(2)">Калибровать</button>
          </div>
        </div>
        
        <div class="servo-card">
          <h3>Servo 3</h3>
          <div class="servo-control">
            <label>Угол: <span id="servo3-value">90</span>°</label>
            <input type="range" id="servo3" min="0" max="180" value="90" oninput="updateServoValue(3, this.value)">
          </div>
          <button class="btn-apply" onclick="setServo(3)">Применить</button>
          <div class="calibrate-section">
            <h4>Калибровка</h4>
            <div class="calibrate-inputs">
              <input type="number" id="servo3-min" placeholder="Min" value="140">
              <input type="number" id="servo3-max" placeholder="Max" value="480">
            </div>
            <button class="btn-calibrate" onclick="calibrateServo(3)">Калибровать</button>
          </div>
        </div>
      </div>
    </div>
    
    <!-- DC Моторы -->
    <div class="panel">
      <h2>🚗 DC Моторы</h2>
      <div class="motor-grid">
        <div class="motor-card">
          <h3>Motor A</h3>
          <div class="motor-controls">
            <button class="motor-btn btn-reverse" onmousedown="startMotor('A', -255)" onmouseup="stopMotor('A')" ontouchstart="startMotor('A', -255)" ontouchend="stopMotor('A')">◀</button>
            <button class="motor-btn btn-stop" onclick="setMotorSpeed('A', 0)">■</button>
            <button class="motor-btn btn-forward" onmousedown="startMotor('A', 255)" onmouseup="stopMotor('A')" ontouchstart="startMotor('A', 255)" ontouchend="stopMotor('A')">▶</button>
          </div>
          <div class="motor-speed">Скорость: <span id="motorA-speed">0</span></div>
        </div>
        
        <div class="motor-card">
          <h3>Motor B</h3>
          <div class="motor-controls">
            <button class="motor-btn btn-reverse" onmousedown="startMotor('B', -255)" onmouseup="stopMotor('B')" ontouchstart="startMotor('B', -255)" ontouchend="stopMotor('B')">◀</button>
            <button class="motor-btn btn-stop" onclick="setMotorSpeed('B', 0)">■</button>
            <button class="motor-btn btn-forward" onmousedown="startMotor('B', 255)" onmouseup="stopMotor('B')" ontouchstart="startMotor('B', 255)" ontouchend="stopMotor('B')">▶</button>
          </div>
          <div class="motor-speed">Скорость: <span id="motorB-speed">0</span></div>
        </div>
      </div>
      
      <div class="motor-actions">
        <button class="btn-stop-all" onclick="stopAllMotors()">🛑 Остановить все моторы</button>
      </div>
    </div>
  </div>
  
  <script>
    const API_BASE = window.location.origin;
    
    // Обновление отображения значения сервопривода
    function updateServoValue(id, value) {
      document.getElementById('servo' + id + '-value').textContent = value;
    }
    
    // Установка угла сервопривода
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
    
    // Калибровка сервопривода
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
    
    // Запуск мотора
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
        }
      } catch (error) {
        showMessage('Ошибка: ' + error, 'error');
      }
    }
    
    // Установка скорости мотора
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
        }
      } catch (error) {
        showMessage('Ошибка: ' + error, 'error');
      }
    }
    
    // Остановка мотора (отпускание кнопки)
    function stopMotor(motor) {
      // Можно оставить скорость или остановить - зависит от логики
      // Сейчас оставляем последнюю установленную скорость
    }
    
    // Остановка всех моторов
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
    
    // Загрузка статуса системы
    async function loadStatus() {
      try {
        const response = await fetch(API_BASE + '/api/status');
        const result = await response.json();
        document.getElementById('robot-ip').textContent = result.ip || 'N/A';
      } catch (error) {
        document.getElementById('robot-ip').textContent = 'Connection error';
      }
    }
    
    // Загрузка текущего состояния сервоприводов
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
    
    // Загрузка текущего состояния моторов
    async function loadMotors() {
      try {
        const response = await fetch(API_BASE + '/api/motor');
        const result = await response.json();
        document.getElementById('motorA-speed').textContent = result.motorA || 0;
        document.getElementById('motorB-speed').textContent = result.motorB || 0;
      } catch (error) {
        console.error('Error loading motors:', error);
      }
    }
    
    // Показ сообщения
    function showMessage(text, type) {
      const existing = document.querySelector('.message');
      if (existing) existing.remove();
      
      const msg = document.createElement('div');
      msg.className = 'message ' + type;
      msg.textContent = text;
      document.body.appendChild(msg);
      
      setTimeout(() => msg.remove(), 3000);
    }
    
    // Инициализация при загрузке
    window.onload = function() {
      loadStatus();
      loadServos();
      loadMotors();
      
      // Периодическое обновление статуса
      setInterval(loadStatus, 5000);
    };
  </script>
</body>
</html>
)rawliteral";
}
