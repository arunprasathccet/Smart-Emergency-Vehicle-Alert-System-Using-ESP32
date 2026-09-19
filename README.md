# 🚑 Smart Emergency Vehicle Alert System Using ESP32

## 📌 Project Overview

The **Smart Emergency Vehicle Alert System** is an embedded prototype designed to help drivers detect nearby emergency vehicles such as ambulances, police vehicles, and fire trucks.

The system uses **two MAX9814 microphone modules** to monitor surrounding sound. An **ESP32** processes the microphone signals and checks for siren-like sound characteristics. When an emergency siren is detected, the ESP32 compares the sound levels from the two microphones to estimate the direction of the emergency vehicle.

The direction and warning status are displayed on a **128×64 OLED display**, while Red and Green LEDs provide visual status indication.

> **Prototype note:** This is an experimental siren-detection prototype. Loud horns, alarms, music, or other tonal sounds may sometimes be detected as a siren. A production automotive system would require more advanced audio classification and calibration.

---

## 🎯 Features

- 🚑 Emergency siren detection
- 🎤 Dual-microphone sound monitoring
- ⬅️ Left direction indication
- ➡️ Right direction indication
- ⬆️ Front direction indication
- 🖥️ OLED warning display
- 🔴 Red LED for emergency detection
- 🟢 Green LED for normal operation
- 📊 Serial Monitor debugging
- ⚙️ Adjustable sound and frequency thresholds
- 🔌 Low-cost ESP32-based prototype

---

## 🧩 System Block Diagram

```text
       Emergency Vehicle Siren
                 │
        ┌────────┴────────┐
        │                 │
        ▼                 ▼
  Left MAX9814       Right MAX9814
   Microphone          Microphone
        │                 │
        └────────┬────────┘
                 ▼
             ┌───────┐
             │ ESP32 │
             │ Sound │
             │Analysis
             └───┬───┘
                 │
       ┌─────────┼─────────┐
       │         │         │
       ▼         ▼         ▼
     OLED     Red LED   Green LED
    Display    Alert     Normal
       │
       ▼
 LEFT / RIGHT / FRONT
```

---

## 🔧 Hardware Components

| S.No | Component | Quantity | Purpose |
|---:|---|---:|---|
| 1 | ESP32 Development Board | 1 | Main controller |
| 2 | MAX9814 Microphone Module | 2 | Left and right sound detection |
| 3 | 128×64 OLED I2C Display | 1 | Emergency and direction display |
| 4 | Red LED | 1 | Emergency indication |
| 5 | Green LED | 1 | Normal-status indication |
| 6 | 220Ω Resistor | 2 | LED current limiting |
| 7 | Breadboard | 1 | Prototype assembly |
| 8 | Jumper Wires | As required | Connections |
| 9 | USB Cable | 1 | ESP32 programming and power |

---

## 🔌 Pin Configuration

| Component | Pin | ESP32 |
|---|---|---|
| Left MAX9814 | OUT | GPIO34 |
| Right MAX9814 | OUT | GPIO35 |
| OLED | SDA | GPIO21 |
| OLED | SCL | GPIO22 |
| Red LED | Anode through 220Ω | GPIO18 |
| Green LED | Anode through 220Ω | GPIO19 |
| MAX9814 | VCC | 3.3V |
| MAX9814 | GND | GND |
| OLED | VCC | 3.3V |
| OLED | GND | GND |

### OLED I2C Address

The current code uses:

```text
0x3C
```

---

## 💻 Software Requirements

### Arduino IDE

Use Arduino IDE with the ESP32 board package installed.

### Required Libraries

Install from Arduino IDE Library Manager:

- **Adafruit GFX Library**
- **Adafruit SSD1306**

### Board

Select:

```text
ESP32 Dev Module
```

---

## ⚙️ Working Principle

1. Two MAX9814 microphones continuously capture surrounding audio.
2. ESP32 samples both microphone signals.
3. The program calculates the audio RMS level and approximate frequency.
4. If the signal meets the configured sound and frequency conditions for several consecutive windows, a possible siren is detected.
5. ESP32 compares the two microphone levels:
   - Left significantly stronger → **LEFT**
   - Right significantly stronger → **RIGHT**
   - Similar levels → **FRONT**
6. The OLED displays the emergency status and direction.
7. Red LED turns ON during emergency detection.
8. Green LED remains ON during normal operation.

---

## 🧪 Testing

For initial testing, play an emergency-vehicle siren from another device.

### Normal

```text
SYSTEM
NORMAL
```

Green LED → ON  
Red LED → OFF

### Siren From Left

```text
EMERGENCY
SIREN
DIRECTION: LEFT
```

### Siren From Right

```text
EMERGENCY
SIREN
DIRECTION: RIGHT
```

### Siren From Front

```text
EMERGENCY
SIREN
IN FRONT
```

---

## 🔧 Calibration

Microphone readings vary with microphone sensitivity, distance, background noise, and speaker volume.

Open Serial Monitor at:

```text
115200 baud
```

Observe:

```text
L_RMS
R_RMS
L_F
R_F
```

The main adjustable parameters are:

```cpp
const float MIN_RMS = 45.0;
const float SIREN_MIN_HZ = 600.0;
const float SIREN_MAX_HZ = 1800.0;
```

If false detections occur, increase `MIN_RMS`. If the test siren is not detected, carefully adjust the threshold after measuring normal background noise.

---

## 📂 Project Structure

```text
Smart-Emergency-Vehicle-Alert-System/
│
├── Smart_Emergency_Vehicle_Alert_System.ino
├── README.md
│
└── images/
    ├── block_diagram.png
    └── circuit_diagram.png
```

---

## 🚀 How to Run

1. Connect the hardware according to the pin table.
2. Install **Adafruit GFX** and **Adafruit SSD1306**.
3. Open `Smart_Emergency_Vehicle_Alert_System.ino`.
4. Select **ESP32 Dev Module**.
5. Select the correct COM port.
6. Upload the program.
7. Open Serial Monitor at **115200 baud**.
8. Test using an emergency-vehicle siren recording.
9. Calibrate the detection threshold if required.

---

## 📈 Advantages

- Low-cost prototype
- Real-time sound monitoring
- Dual-microphone direction estimation
- OLED visual indication
- Simple embedded architecture
- Easy to demonstrate and extend
- Suitable for embedded-systems learning

---

## 🌍 Applications

- Passenger vehicles
- Public transport
- Smart transportation prototypes
- Automotive safety research
- Emergency-awareness systems
- Academic embedded-system projects

---

## 🔮 Future Scope

### 🤖 AI-Based Siren Classification

A trained audio model can distinguish:

- Ambulance siren
- Police siren
- Fire-truck siren
- Car horn
- Music
- Traffic noise

### 📍 Distance Estimation

The system can classify the emergency vehicle as:

```text
FAR → NEAR → VERY NEAR
```

### 🎵 Automatic Music Control

An audio-processing stage can be added so that:

```text
Normal       → Music normal
Siren near   → Music volume reduced
Very near    → Music muted
Vehicle gone → Music restored
```

### 🧭 Improved Direction Detection

Additional microphones and time-difference-of-arrival (TDOA) processing can provide more accurate direction estimation.

### 🚗 Automotive Integration

The prototype can later be integrated with an automotive audio system using a properly designed audio interface and vehicle-safe power supply.

---

## 👨‍💻 Skills Demonstrated

- Embedded C/C++
- ESP32 programming
- GPIO
- ADC
- I2C communication
- OLED interfacing
- Analog signal acquisition
- Basic digital signal processing
- Microphone interfacing
- Real-time embedded programming
- Hardware debugging

---

## 📜 License

This project is intended for educational and prototype development purposes. You may modify and extend the source code for academic and personal projects.

---

## 👤 Author

**Arunprasath B**

**Project:** Smart Emergency Vehicle Alert System Using ESP32

**Domain:** Embedded Systems / IoT / Automotive Safety
