# Personal-Assistant-Robot (Stage 1)
This project is a Personal Assistant Robot(Stage-1) built using an Evoed Arduino board. It is designed to perform various tasks through voice commands and mobile control via Bluetooth. The robot also includes an ESP32-CAM module for real-time video streaming. This combination of voice, Bluetooth, and camera features makes it an intelligent and interactive robot capable of assisting users in daily activities.

**NEW**
# 🤖 Autonomous ESP32 Hotel Food Delivery & Plate Pickup Robot (Stage 2)

An advanced, multi-functional autonomous service robot powered by the ESP32 microcontroller. Serving as **Stage 2** in this robotics evolution (following a Bluetooth-controlled Personal Assistant Robot Stage 1), this project upgrades from basic wireless control to full IoT automation, featuring a responsive web-based ordering and pickup dashboard, a dual-direction line-following system, expanded Finite State Machine (FSM) architecture, and integrated audio announcements via I2S and LittleFS.

---

## 🌟 Key Features

* **📱 Web Ordering & Pickup Dashboard:** Integrated Wi-Fi Access Point and HTTP web server allowing customers and staff to order menu items (Pizza, Burger, Donut) or summon the robot for empty plate collection from any smartphone or tablet.
* **🔄 Expanded Finite State Machine (FSM):** Manages two complete workflows with robust state transitions:
* *Delivery Workflow:* `WAIT_FOR_ORDER` $\rightarrow$ `WAIT_FOR_FOOD` $\rightarrow$ `DELIVER_FORWARD` $\rightarrow$ `WAIT_AT_TABLE` $\rightarrow$ `RETURN_BACKWARD` $\rightarrow$ `WAIT_FOR_ORDER`
* *Plate Pickup Workflow:* `WAIT_FOR_ORDER` $\rightarrow$ `PICKUP_FORWARD` $\rightarrow$ `WAIT_AT_TABLE_PLATE` $\rightarrow$ `PICKUP_RETURN` $\rightarrow$ `WAIT_AT_STATION_PLATE` $\rightarrow$ `WAIT_FOR_ORDER`


* **🔊 I2S Voice & Audio Announcements:** Streams high-quality MP3 voice prompts (`robofood.mp3`, `welcome.mp3`, `enjoyfood.mp3`) stored in internal LittleFS flash memory using the ESP32's internal DAC and an external PAM8403 amplifier.
* **🛑 Safety & Abort System:**
* *Ultrasonic Obstacle Scanning:* Scans via an HC-SR04 sensor and halts motion immediately if an obstacle is detected within $\le 10\text{ cm}$.
* *Mid-Transit Abort:* Automatically aborts delivery and returns to the station if food is removed from the tray prematurely.


* **📟 Real-time Status Updates:** Displays live operational status on an I2C 16x2 LCD screen and streams real-time JSON updates to the web dashboard.

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 Development Board (38-pin or similar)
* **Motor Driver:** L298N Dual H-Bridge Motor Driver
* **Motors & Wheels:** 4-Wheel Drive setup using DC Gear Motors (300 RPM) with 7 cm wheels
* **Sensors:**
* 4× IR Line Tracking Sensors (2 Front for delivery, 2 Back for reverse tracking/station return)
* 1× IR Proximity Sensor (Food Tray / Plate Detection)
* 1× HC-SR04 Ultrasonic Sensor (Obstacle Avoidance)


* **Audio Module:** Internal DAC pins + PAM8403 Amplifier + 3W Speaker
* **Display:** 16x2 LCD with I2C Backpack (Address `0x27`)
* **Power Supply:** 12V Li-ion Battery pack (for motors) + 5V Buck Converter (for ESP32 and logic)

---

## 📌 Wiring & Pin Map

| Component | Pin Function | ESP32 GPIO |
| --- | --- | --- |
| **Audio (I2S)** | Data Out (DOUT) <br>

<br> Bit Clock (BCLK) <br>

<br> Word Select (LRCK) | GPIO 25 <br>

<br> GPIO 27 <br>

<br> GPIO 26 |
| **L298N Motors** | Left Motor (ENA, IN1, IN2) <br>

<br> Right Motor (ENB, IN3, IN4) | GPIO 13, 12, 15 <br>

<br> GPIO 14, 32, 33 |
| **Line Sensors** | Front Left / Right IR <br>

<br> Back Left / Right IR | GPIO 18, 19 <br>

<br> GPIO 16, 17 |
| **Tray & Distance** | Food/Plate IR Sensor <br>

<br> Ultrasonic (TRIG / ECHO) | GPIO 23 <br>

<br> GPIO 5 / GPIO 4 |
| **I2C Display** | LCD SDA / SCL | GPIO 21 / GPIO 22 |

---

## 🚀 Getting Started & Installation

### 1. Prerequisites & Libraries

Install the following libraries in your Arduino IDE:

* `Audio.h` (ESP32-audioI2S)
* `LiquidCrystal_I2C`
* `WiFi.h`, `WebServer.h`, `Wire.h`, `LittleFS.h`

### 2. Flash Memory Setup (LittleFS)

1. Create a `data` directory inside your Arduino sketch folder.
2. Add your voice clips: `robofood.mp3`, `welcome.mp3`, and `enjoyfood.mp3`.
3. Upload the files to the ESP32 using the **ESP32 Sketch Data Upload** tool in Arduino IDE.

### 3. Firmware Upload

1. Open the `.ino` sketch in Arduino IDE.
2. Select Partition Scheme: **Default 4MB with spiffs (1.2MB APP / 1.5MB SPIFFS)**.
3. Select your ESP32 COM port and click **Upload**.

---

## 📱 How to Use

1. **Power on** the robot. It will play a boot voice prompt and launch a Wi-Fi Access Point:
* **SSID:** `Hotel_Food_Delivery_Bot`
* **Password:** `12345678`


2. **Connect** your smartphone or tablet to the robot's Wi-Fi network.
3. **Open your browser** and navigate to `[http://192.168.4.1](http://192.168.4.1)`.
4. **Operate:**
* Place an order on the dashboard and load food onto the tray—the robot will automatically detect the food and navigate to the table.
* Alternatively, use the **Request Plate Pickup** button to summon the robot back to the table for clearing dishes.
