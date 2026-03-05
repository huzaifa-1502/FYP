

#include <WiFi.h>
#include <WebServer.h>

/* ================= PINS ================= */
#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16
#define ENA 4
#define ENB 5
#define BUZZER 17

#define LED_LEFT 20
#define LED_RIGHT 21

#define SPEED 200

/* ================= STATE ================= */
enum CarState { AWAKE, DROWSY };
CarState carState = AWAKE;

/* Direction states for LED indicators */
enum DirectionState { NONE, LEFT, RIGHT };
DirectionState dirState = NONE;

WebServer server(80);

/* ================= MOTOR ================= */
void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  dirState = NONE; // stop direction blinking
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  dirState = NONE;
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  dirState = NONE;
}

void moveLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  dirState = LEFT;
}

void moveRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  dirState = RIGHT;
}

void gradualBrake() {
  for (int sp = SPEED; sp > 0; sp -= 2) {
    ledcWrite(0, sp);
    ledcWrite(1, sp);
    delay(25);
  }
  stopCar();
  ledcWrite(0, SPEED);
  ledcWrite(1, SPEED);
  digitalWrite(BUZZER, LOW);
  carState = AWAKE;
}

/* ================= LED BLINK HANDLER ================= */
bool ledState = false;
unsigned long lastBlink = 0;
const unsigned long blinkInterval = 400; // 400ms

void handleLEDs() {
  unsigned long now = millis();

  // Blink logic only every blinkInterval
  if (now - lastBlink >= blinkInterval) {
    lastBlink = now;
    ledState = !ledState;

    // Drowsy: both LEDs blink
    if (carState == DROWSY) {
      digitalWrite(LED_LEFT, ledState);
      digitalWrite(LED_RIGHT, ledState);
    } 
    // Direction indicators
    else if (dirState == LEFT) {
      digitalWrite(LED_LEFT, ledState);
      digitalWrite(LED_RIGHT, LOW);
    } 
    else if (dirState == RIGHT) {
      digitalWrite(LED_LEFT, LOW);
      digitalWrite(LED_RIGHT, ledState);
    } 
    else { // normal
      digitalWrite(LED_LEFT, LOW);
      digitalWrite(LED_RIGHT, LOW);
    }
  }
}

/* ================= UI ================= */
String page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>AI Drowsiness Detection Car</title>

<style>
body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: radial-gradient(circle at top, #16222A, #0f2027);
  color: white;
  text-align: center;
  overflow: hidden;
}

/* ===== HEADER ===== */
h2 {
  margin: 10px;
  text-shadow: 0 0 12px cyan;
}

/* ===== STATUS ===== */
.status {
  font-size: 20px;
  margin: 10px;
  padding: 8px 20px;
  display: inline-block;
  border-radius: 20px;
  box-shadow: 0 0 15px;
}
.awake {
  background: #00c853;
  box-shadow: 0 0 15px #00e676;
}
.drowsy {
  background: red;
  animation: blink 1s infinite;
}

@keyframes blink {
  0% { opacity: 1; }
  50% { opacity: 0.3; }
  100% { opacity: 1; }
}

/* ===== ROAD ===== */
.road {
  position: relative;
  width: 100%;
  height: 230px;
  background: #333;
  border-top: 4px solid #555;
  border-bottom: 4px solid #555;
}

.road::before {
  content: '';
  position: absolute;
  top: 50%;
  width: 100%;
  height: 6px;
  background: repeating-linear-gradient(
    to right,
    white 0,
    white 40px,
    transparent 40px,
    transparent 80px
  );
  animation: moveRoad 0.8s linear infinite;
}

@keyframes moveRoad {
  from { background-position-x: 0; }
  to { background-position-x: -80px; }
}

/* ===== CAR ===== */
.car {
  position: absolute;
  bottom: 20px;
  left: 50%;
  transform: translateX(-50%);
  font-size: 80px;
  transition: all 0.4s ease;
  filter: drop-shadow(0 0 15px cyan);
}

/* ===== CONTROLS ===== */
.panel {
  margin-top: 20px;
}

button {
  padding: 16px 35px;
  margin: 6px;
  font-size: 18px;
  border: none;
  border-radius: 14px;
  cursor: pointer;
  color: white;
  box-shadow: 0 0 15px rgba(0,0,0,0.6);
}

button:hover {
  transform: scale(1.05);
}

#forward { background:#00c853; }
#back { background:#d50000; }
#left { background:#2962ff; }
#right { background:#ff6d00; }
#stop { background:black; }
#reset { background:pink; color:black; }

/* ===== ALERT ===== */
.alert {
  margin-top: 15px;
  font-size: 16px;
  color: #ff5252;
  display: none;
  animation: blink 1s infinite;
}
</style>
</head>

<body>

<h2>🚗 AI Drowsiness Detection Smart Car</h2>

<div id="driverStatus" class="status awake">Driver Status: AWAKE</div>

<div class="road">
  <div class="car" id="car">🚗</div>
</div>

<div class="alert" id="alertText">
  ⚠ DRIVER DROWSY! BUZZER & LED ACTIVATED ⚠
</div>

<div class="panel">
  <button id="forward" onclick="moveCar('forward')">⬆ Forward</button><br>
  <button id="left" onclick="moveCar('left')">⬅ Left</button>
  <button id="right" onclick="moveCar('right')">Right ➡</button><br>
  <button id="back" onclick="moveCar('back')">⬇ Backward</button><br>
  <button id="stop" onclick="emergencyStop()">🛑 Emergency Brake</button>
  <button id="reset" onclick="resetSystem()">🔄 Reset</button>
</div>

<script>
let car = document.getElementById("car");
let statusBox = document.getElementById("driverStatus");
let alertText = document.getElementById("alertText");

let posX = 50;
let posY = 0;
let drowsy = false;

/* ===== CAR MOVEMENT ===== */
function moveCar(dir) {
  if (drowsy) return;

  if(dir === 'forward'){ fetch('/forward'); posY += 10; }
  if(dir === 'back'){ fetch('/back'); posY -= 10; }
  if(dir === 'left'){ fetch('/left'); posX -= 5; }
  if(dir === 'right'){ fetch('/right'); posX += 5; }

  car.style.left = posX + "%";
  car.style.bottom = 20 + posY + "px";
}

/* ===== DROWSINESS DETECTED (CALL FROM AI) ===== */
function drowsinessDetected() {
  drowsy = true;
  fetch('/stop');

  statusBox.innerHTML = "Driver Status: DROWSY";
  statusBox.className = "status drowsy";

  alertText.style.display = "block";
  car.style.filter = "drop-shadow(0 0 25px red)";
}

/* ===== EMERGENCY STOP ===== */
function emergencyStop() {
  fetch('/stop');
  car.style.filter = "drop-shadow(0 0 30px red)";
}

/* ===== RESET SYSTEM ===== */
function resetSystem() {
  fetch('/awake');
  drowsy = false;

  statusBox.innerHTML = "Driver Status: AWAKE";
  statusBox.className = "status awake";

  alertText.style.display = "none";
  car.style.filter = "drop-shadow(0 0 15px cyan)";

  posX = 50;
  posY = 0;
  car.style.left = "50%";
  car.style.bottom = "20px";
}

/* ===== DEMO AUTO DROWSY (REMOVE IN FINAL) ===== */
// setTimeout(drowsinessDetected, 8000);
</script>

</body>
</html>
)rawliteral";

/* ================= SETUP ================= */
void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_LEFT, OUTPUT); pinMode(LED_RIGHT, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW);

  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);
  ledcAttachPin(ENA, 0);
  ledcAttachPin(ENB, 1);

  ledcWrite(0, SPEED);
  ledcWrite(1, SPEED);

  stopCar();

  WiFi.softAP("ESP32-CAR");

  server.on("/", []() { server.send(200, "text/html", page); });

  /* ===== MOVEMENT (BLOCKED ONLY DURING DROWSY) ===== */
  server.on("/forward", [](){ if(carState==AWAKE) moveForward(); });
  server.on("/back", [](){ if(carState==AWAKE) moveBackward(); });
  server.on("/left", [](){ if(carState==AWAKE) moveLeft(); });
  server.on("/right", [](){ if(carState==AWAKE) moveRight(); });
  server.on("/stop", [](){ if(carState==AWAKE) stopCar(); });

  /* ===== AI SIGNALS ===== */
  server.on("/drowsy", [](){
    if(carState==AWAKE){
      carState = DROWSY;
      digitalWrite(LED_LEFT, HIGH);
       digitalWrite(LED_RIGHT, HIGH);
      digitalWrite(BUZZER, HIGH);
      gradualBrake();
    }
  });

  server.on("/awake", [](){
    carState = AWAKE;
    digitalWrite(BUZZER, LOW);
    stopCar();
  });

  server.begin();
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();
  handleLEDs(); // handle LED blinking
}