🚗 Driver Drowsiness Detection System — IoT Based
Real-Time Driver Drowsiness Detection using Computer Vision & IoT
University of Karachi — Department of Computer Science
Session: December 2025

📌 Overview
An IoT-based robot car system that detects driver drowsiness in real-time using computer vision and responds by triggering a gradual braking mechanism — simulating human reaction to fatigue. The system monitors facial features like eye closure and blinking rate using a camera, classifies alertness using a CNN model, and communicates wirelessly with an ESP32 microcontroller to control the vehicle.

🧠 Tech Stack
LayerTechnologyLanguagePythonComputer VisionOpenCV (Haar Cascade)Deep LearningTensorFlow / Keras (Custom CNN)MicrocontrollerESP32Motor DriverL298NPower Supply3x 18650 Li-ion Cells (11V–12V)CommunicationWiFi (HTTP)Alert SystemPygame (buzzer sound)UI DashboardHTML + CSS + JavaScript

⚙️ How It Works

Camera captures live video feed of the driver's face
Haar Cascade detects face, left eye, and right eye regions
CNN Model (custmodel.h5) classifies each eye as Open or Closed
A drowsiness counter increments when both eyes are detected closed
If drowsiness persists beyond threshold:

🔔 Alarm sound triggers
📡 HTTP request sent to ESP32 (/drowsy endpoint)
🚗 ESP32 activates gradual braking (PWM from 255 → 70 → 0)
💡 LEDs blink and buzzer activates


When driver is awake, /awake signal resets the system


🔧 Hardware Components

ESP32 Microcontroller
L298N Motor Driver Module
3x 18650 Li-ion Battery Cells
IR / Camera Module
Buzzer
Left & Right LEDs
Robot Car Chassis with DC Motors


📁 Project Structure
├── drowsiness_detection.py   # Main Python script
├── models/
│   └── custmodel.h5          # Trained CNN model
├── haar cascade files/
│   ├── haarcascade_frontalface_alt.xml
│   ├── haarcascade_lefteye_2splits.xml
│   └── haarcascade_righteye_2splits.xml
├── alarm.wav                 # Alert sound
├── image.jpg                 # Snapshot on drowsiness
└── ESP32_Firmware/
    └── firmware.ino          # Arduino C++ ESP32 code

🚀 Getting Started
Prerequisites
bashpip install opencv-python tensorflow keras numpy pygame requests
Run
bashpython drowsiness_detection.py
```

### ESP32 Setup
1. Flash `firmware.ino` via Arduino IDE onto the ESP32
2. ESP32 creates a WiFi Access Point: **`ESP32-CAR`**
3. Connect your PC to `ESP32-CAR` network
4. Update `ESP32_IP = "192.168.4.1"` in the Python script if needed

---

## 📊 Detection Logic
```
Both Eyes Closed → time += 1
Both Eyes Open  → time -= 1
time > 3        → DROWSY ALERT TRIGGERED
time < 0        → Reset to 0

🌐 ESP32 API Endpoints
EndpointAction/Serves Web Dashboard/drowsyTriggers gradual brake + buzzer + LEDs/awakeResets system to AWAKE state/forwardMove forward (blocked if DROWSY)/backMove backward/leftTurn left/rightTurn right/stopStop car

🎯 Detection Method
Behavioral Analysis (EAR — Eye Aspect Ratio): Non-intrusive, low-cost, and high accuracy for real-time applications using facial landmark detection.
