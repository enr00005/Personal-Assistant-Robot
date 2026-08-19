# Personal-Assistant-Robot (Stage 1)
This project is a Personal Assistant Robot(Stage-1) built using an Evoed Arduino board. It is designed to perform various tasks through voice commands and mobile control via Bluetooth. The robot also includes an ESP32-CAM module for real-time video streaming. This combination of voice, Bluetooth, and camera features makes it an intelligent and interactive robot capable of assisting users in daily activities.

**NEW**
# 🤖 Autonomous ESP32 Hotel Food Delivery Robot (Stage 2)

An autonomous, multi-functional food delivery robot powered by the ESP32 microcontroller. The system features a **Wi-Fi Web Dashboard** for real-time ordering, a **5-stage Finite State Machine (FSM)** for dual-direction line-following, an **I2S MP3 audio announcement system** reading from **LittleFS**, and integrated obstacle avoidance.

---

## 🌟 Key Features

* 📱 **Web Ordering Dashboard:** Integrated Wi-Fi Access Point and HTTP web server allowing customers to order food items (Pizza, Burger, Donut) from any smartphone or tablet.
* 🔄 **5-Stage Finite State Machine (FSM):** Handles state transitions safely:
  1. `WAIT_FOR_ORDER` (Station Idle)
  2. `WAIT_FOR_FOOD` (Kitchen Loading Detection)
  3. `DELIVER_FORWARD` (Forward Line Following)
  4. `WAIT_AT_TABLE` (Customer Pickup & Voice Greeting)
  5. `RETURN_BACKWARD` (Reverse Line Following to Station)
* 🔊 **I2S Voice & Audio Announcements:** Streams high-quality MP3 voice prompts (`robofood.mp3`, `welcome.mp3`, `enjoyfood.mp3`) stored in the internal **LittleFS** flash memory via I2S.
* 🛑 **Safety & Abort System:** 
  * **Ultrasonic Distance Scanning:** Detects obstacles within $\le 10\text{ cm}$ and halts motion in both forward and reverse directions.
  * **Mid-Transit Abort:** Immediately returns to the kitchen if food is removed from the tray before arriving at the table.
* 📟 **Real-time Status Updates:** Displays real-time operational status on an I2C 16x2 LCD screen and streams JSON status updates to the web UI.

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 Development Board
* **Motor Driver:** L298N Dual H-Bridge Motor Driver
* **Motors:** 2x DC Gear Motors with Wheels
* **Sensors:**
  * 4x IR Line Tracking Sensors (2 Front, 2 Back)
  * 1x IR Proximity Sensor (Food Tray Detection)
  * 1x HC-SR04 Ultrasonic Sensor (Obstacle Avoidance)
* **Audio:** I2S Audio Module / DAC (or internal DAC) + PAM8403 Amplifier + 3W Speaker
* **Display:** 16x2 LCD with I2C Backpack (`0x27`)
* **Power Supply:** 12V Li-ion Battery pack (for motors) + 5V Buck Converter (for ESP32 & sensors)

---

## 📌 Wiring & Pin Map

| Component | Pin Function | ESP32 GPIO |
| :--- | :--- | :--- |
| **Audio (I2S)** | Data Out (DOUT) | **GPIO 25** |
| | Bit Clock (BCLK) | **GPIO 27** |
| | Word Select (LRCK) | **GPIO 26** |
| **L298N Motors** | Left Motor (ENA, IN1, IN2) | **GPIO 13, 12, 15** |
| | Right Motor (ENB, IN3, IN4) | **GPIO 14, 32, 33** |
| **Line Sensors** | Front Left / Right IR | **GPIO 18, 19** |
| | Back Left / Right IR | **GPIO 16, 17** |
| **Tray & Distance**| Food Tray IR Sensor | **GPIO 23** |
| | Ultrasonic (TRIG / ECHO) | **GPIO 5 / GPIO 4** |
| **I2C Display** | LCD SDA / SCL | **GPIO 21 / GPIO 22** |

---

## 🚀 Getting Started

### 1. Prerequisites & Libraries
Install the following libraries in your Arduino IDE:
* [Audio.h](https://github.com/schreibfaul1/ESP32-audioI2S) (ESP32-audioI2S)
* `LiquidCrystal_I2C`
* `WiFi.h`, `WebServer.h`, `Wire.h`, `LittleFS.h`

### 2. Flash Memory Setup (LittleFS)
1. Create a `data` directory inside your sketch folder.
2. Add your voice clips: `robofood.mp3`, `welcome.mp3`, and `enjoyfood.mp3`.
3. Upload the files to the ESP32 using the **ESP32 LittleFS Filesystem Upload** tool.

### 3. Firmware Upload
1. Open the `.ino` sketch in Arduino IDE.
2. Select **Partition Scheme:** `Default 4MB with spiffs (1.2MB APP / 1.5MB SPIFFS)`.
3. Select your ESP32 COM port and click **Upload**.

---

## 📱 How to Use

1. Power on the robot. It will announce initialization via audio and launch a Wi-Fi Access Point:
   * **SSID:** `Hotel_Food_Delivery_Bot`
   * **Password:** `12345678`
2. Connect your smartphone to the Wi-Fi network and navigate to `http://192.168.4.1` in your browser.
3. Place an order on the dashboard.
4. Place the food tray on the robot — it will automatically detect the food and navigate to the destination table!
