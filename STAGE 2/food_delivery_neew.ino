#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "LittleFS.h"
#include "Audio.h"

// ================= AUDIO CONFIGURATION =================
Audio audio;

void playVoice(const char* filename) {
  Serial.print("Playing Audio: ");
  Serial.println(filename);
  audio.connecttoFS(LittleFS, filename);
}

// Track audio trigger events so they only fire once per state transition
bool welcomePlayed = false;

// ================= LCD CONFIGURATION =================
LiquidCrystal_I2C lcd(0x27, 16, 2); 

String lastLcdLine1 = "";
String lastLcdLine2 = "";

void updateLCD(String line1, String line2) {
  if (line1 != lastLcdLine1 || line2 != lastLcdLine2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    lastLcdLine1 = line1;
    lastLcdLine2 = line2;
  }
}

// ================= WI-FI ACCESS POINT CONFIG =================
const char* ssid = "Hotel_Food_Delivery_Bot";
const char* password = "12345678";

WebServer server(80);

// ================= REASSIGNED PIN DEFINITIONS =================
// Left Motor (L298N) - Reassigned away from I2S Pins (25, 26, 27)
#define ENA 13
#define IN1 12
#define IN2 15

// Right Motor (L298N)
#define ENB 14
#define IN3 32
#define IN4 33

// Front IR Sensors
#define FRONT_LEFT_IR_PIN   18
#define FRONT_RIGHT_IR_PIN  19

// Back IR Sensors
#define BACK_LEFT_IR_PIN    16
#define BACK_RIGHT_IR_PIN   17

// Food Sensor
#define FOOD_IR_PIN         23

// Ultrasonic Sensor Pins (HC-SR04)
#define TRIG_PIN            5
#define ECHO_PIN            4

// ================= CONFIGURATION =================
#define BASE_SPEED     120
#define TURN_SPEED     160

#define LINE_DETECTED HIGH 
#define FOOD_DETECTED LOW    

#define OBSTACLE_DISTANCE_CM 10 

// ================= ORDER & STATE VARIABLES =================
bool isOrderPlaced = false;
String currentOrderDetails = "No active order";

enum RobotState {
  WAIT_FOR_ORDER,    
  WAIT_FOR_FOOD,     
  DELIVER_FORWARD,   
  WAIT_AT_TABLE,     
  RETURN_BACKWARD,
  PICKUP_FORWARD,        // Going to table to collect empty plates
  WAIT_AT_TABLE_PLATE,   // Waiting at table for plate to be placed on tray
  PICKUP_RETURN,         // Returning to station with plates for washing
  WAIT_AT_STATION_PLATE  // Waiting at station for staff to take plate off tray
};

RobotState currentState = WAIT_FOR_ORDER;

// ================= FUNCTION PROTOTYPES =================
void moveForward();
void moveBackward();
void turnLeftForward();
void turnRightForward();
void turnLeftBackward();
void turnRightBackward();
void stopMotors();
void followLineForward();
void followLineBackward();
bool isObstacleDetected();
void handleRoot();
void handleOrder();
void handlePickup();
void handleStatus();

// ================= HTML WEB DASHBOARD =================
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Grand Hotel Food Ordering</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f4f6f9; margin: 0; padding: 20px; text-align: center; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 25px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
    h1 { color: #d9534f; margin-bottom: 5px; }
    h2 { color: #333; font-size: 1.2rem; margin-bottom: 25px; border-bottom: 2px solid #eee; padding-bottom: 10px; }
    .item-card { display: flex; justify-content: space-between; align-items: center; background: #fafafa; padding: 12px 15px; margin-bottom: 12px; border-radius: 8px; border: 1px solid #e2e2e2; }
    .item-info { text-align: left; }
    .item-name { font-weight: bold; font-size: 1.1rem; color: #2c3e50; }
    .item-price { color: #27ae60; font-weight: 600; }
    input[type=number] { width: 55px; padding: 6px; border: 1px solid #ccc; border-radius: 5px; text-align: center; font-size: 1rem; }
    .btn-order { width: 100%; background: #27ae60; color: white; padding: 14px; border: none; border-radius: 8px; font-size: 1.1rem; font-weight: bold; cursor: pointer; margin-top: 15px; transition: 0.2s; }
    .btn-order:hover { background: #219150; }
    .btn-pickup { width: 100%; background: #e67e22; color: white; padding: 14px; border: none; border-radius: 8px; font-size: 1.1rem; font-weight: bold; cursor: pointer; margin-top: 10px; transition: 0.2s; }
    .btn-pickup:hover { background: #d35400; }
    .status-box { margin-top: 25px; padding: 15px; background: #eef7ff; border-radius: 8px; border-left: 5px solid #3498db; text-align: left; }
    .status-title { font-weight: bold; color: #2980b9; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Grand Hotel</h1>
    <h2>Order your food or dish</h2>
    
    <div class="item-card">
      <div class="item-info">
        <div class="item-name">Pizza</div>
        <div class="item-price">$12.00</div>
      </div>
      <input type="number" id="pizzaQty" value="0" min="0" max="10">
    </div>

    <div class="item-card">
      <div class="item-info">
        <div class="item-name">Burger</div>
        <div class="item-price">$8.00</div>
      </div>
      <input type="number" id="burgerQty" value="0" min="0" max="10">
    </div>

    <div class="item-card">
      <div class="item-info">
        <div class="item-name">Donut</div>
        <div class="item-price">$4.00</div>
      </div>
      <input type="number" id="donutQty" value="0" min="0" max="10">
    </div>

    <button class="btn-order" onclick="placeOrder()">Place Order</button>
    <button class="btn-pickup" onclick="requestPickup()">Request Plate Pickup</button>

    <div class="status-box">
      <div class="status-title">Order Status:</div>
      <div id="statusText">Checking status...</div>
      <div id="orderDetailsText" style="margin-top:5px; color:#555;"></div>
    </div>
  </div>

  <script>
    function placeOrder() {
      let pizza = document.getElementById('pizzaQty').value;
      let burger = document.getElementById('burgerQty').value;
      let donut = document.getElementById('donutQty').value;

      if (pizza == 0 && burger == 0 && donut == 0) {
        alert("Please select at least 1 item to place an order.");
        return;
      }

      fetch(`/order?pizza=${pizza}&burger=${burger}&donut=${donut}`)
        .then(res => res.text())
        .then(msg => {
          alert(msg);
          checkStatus();
        });
    }

    function requestPickup() {
      fetch('/pickup')
        .then(res => res.text())
        .then(msg => {
          alert(msg);
          checkStatus();
        });
    }

    function checkStatus() {
      fetch('/status')
        .then(res => res.json())
        .then(data => {
          document.getElementById('statusText').innerText = data.robotState;
          document.getElementById('orderDetailsText').innerText = "Current Order: " + data.orderDetails;
        });
    }

    setInterval(checkStatus, 2000);
    checkStatus();
  </script>
</body>
</html>
)rawliteral";

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // 1. Initialize Storage & Audio System
  if (!LittleFS.begin(true)) {
    Serial.println("Error mounting LittleFS!");
  } else {
    Serial.println("LittleFS mounted successfully.");
  }

  // Audio pin configuration: BCLK=27, LRCK=26, DOUT=25 (Internal DAC)
  audio.setPinout(27, 26, 25, true);
  audio.setVolume(2); // Low clean output level for PAM8403

  // 2. Play boot voice prompt immediately on power-on
  playVoice("/robofood.mp3");

  // 3. Initialize LCD Screen
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Hi! I'm your");
  lcd.setCursor(0, 1);
  lcd.print("Food Delivery");
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Delivery Robot");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(1000);

  // 4. Pin Modes Initialization
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(FRONT_LEFT_IR_PIN, INPUT);
  pinMode(FRONT_RIGHT_IR_PIN, INPUT);
  pinMode(BACK_LEFT_IR_PIN, INPUT);
  pinMode(BACK_RIGHT_IR_PIN, INPUT);
  pinMode(FOOD_IR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  stopMotors();

  // 5. Start Access Point
  WiFi.softAP(ssid, password);
  Serial.println("\n--- Wi-Fi Started ---");
  Serial.print("Access Web Dashboard at: http://"); 
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/order", handleOrder);
  server.on("/pickup", handlePickup);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("Robot Initialized. Ready at station.");
}

// ================= MAIN LOOP =================
void loop() {
  // Required: keeps audio stream decoded
  audio.loop();

  // Process incoming HTTP requests
  server.handleClient();

  bool foodPresent  = (digitalRead(FOOD_IR_PIN) == FOOD_DETECTED);
  bool frontLeft    = (digitalRead(FRONT_LEFT_IR_PIN) == LINE_DETECTED);
  bool frontRight   = (digitalRead(FRONT_RIGHT_IR_PIN) == LINE_DETECTED);
  bool backLeft     = (digitalRead(BACK_LEFT_IR_PIN) == LINE_DETECTED);
  bool backRight    = (digitalRead(BACK_RIGHT_IR_PIN) == LINE_DETECTED);

  switch (currentState) {

    case WAIT_FOR_ORDER:
      stopMotors();
      updateLCD("Station Idle", "Wait for order..");
      welcomePlayed = false; // Reset trigger for next delivery run
      break;

    case WAIT_FOR_FOOD:
      stopMotors();
      updateLCD("Order Received!", "Place Food On Tray");
      if (foodPresent) {
        Serial.println("Food detected on tray! Starting delivery...");
        updateLCD("Food Detected!", "Starting Soon...");
        delay(1000);
        currentState = DELIVER_FORWARD;
      }
      break;

    case DELIVER_FORWARD:
      if (!foodPresent) {
        Serial.println("Food signal lost mid-transit! Aborting...");
        stopMotors();
        updateLCD("Food Removed!", "Returning Home..");
        delay(500);
        currentState = RETURN_BACKWARD;
        break;
      }

      if (isObstacleDetected()) {
        stopMotors();
        updateLCD("Obstacle Ahead!", "Waiting...");
        delay(2000);
        break;
      }

      if (frontLeft && frontRight) {
        Serial.println("Reached Table! Stopping for delivery.");
        stopMotors();
        updateLCD("Arrived at Table", "Pick up food!");
        currentState = WAIT_AT_TABLE;
      } else {
        updateLCD("Delivering Food", "Moving to Table");
        followLineForward();
      }
      break;

    case WAIT_AT_TABLE:
      stopMotors();
      updateLCD("Arrived at Table", "Pick up food!");

      // Play arrival audio sequence once when stopping at table
      if (!welcomePlayed) {
        welcomePlayed = true;
        playVoice("/welcome.mp3");
      }

      if (!foodPresent) {
        Serial.println("Food picked up by customer!");
        updateLCD("Food Picked Up!", "Returning soon");
        delay(2000);
        currentState = RETURN_BACKWARD;
      }
      break;

    case RETURN_BACKWARD:
      if (isObstacleDetected()) {
        stopMotors();
        updateLCD("Obstacle Behind!", "Waiting...");
        delay(2000);
        break;
      }

      if (backLeft && backRight) {
        Serial.println("Reached Station!");
        stopMotors();
        updateLCD("Back at Station", "Resetting...");
        delay(1000);
        isOrderPlaced = false;
        currentOrderDetails = "No active order";
        currentState = WAIT_FOR_ORDER;
      } else {
        updateLCD("Returning Home", "Moving Backward");
        followLineBackward();
      }
      break;

    // ================= PLATE PICKUP STATES =================
    case PICKUP_FORWARD:
      if (isObstacleDetected()) {
        stopMotors();
        updateLCD("Obstacle Ahead!", "Waiting...");
        delay(2000);
        break;
      }

      if (frontLeft && frontRight) {
        Serial.println("Reached Table for Pickup!");
        stopMotors();
        updateLCD("Pickup Arrived", "Place Plate on Tray");
        currentState = WAIT_AT_TABLE_PLATE;
      } else {
        updateLCD("Going for Pickup", "Moving to Table");
        followLineForward();
      }
      break;

    case WAIT_AT_TABLE_PLATE:
      stopMotors();
      updateLCD("Pickup Arrived", "Place Plate on Tray");

      if (foodPresent) {
        Serial.println("Plate placed on tray! Returning to station...");
        updateLCD("Plate Collected!", "Returning Home..");
        delay(1500);
        currentState = PICKUP_RETURN;
      }
      break;

    case PICKUP_RETURN:
      if (isObstacleDetected()) {
        stopMotors();
        updateLCD("Obstacle Behind!", "Waiting...");
        delay(2000);
        break;
      }

      if (backLeft && backRight) {
        Serial.println("Reached Station with Plate!");
        stopMotors();
        // FIXED: Transition to WAIT_AT_STATION_PLATE so it stays stopped 
        // and doesn't fall back into line tracking due to momentum drift.
        currentState = WAIT_AT_STATION_PLATE;
      } else {
        updateLCD("Returning w/ Plate", "Moving Backward");
        followLineBackward();
      }
      break;

    case WAIT_AT_STATION_PLATE:
      stopMotors();
      updateLCD("Back at Station", "Take Plate off Tray");

      // Wait until staff takes the plate off for washing (!foodPresent)
      if (!foodPresent) {
        Serial.println("Plate taken by staff for washing!");
        updateLCD("Plate Collected", "Resetting...");
        delay(1000);
        isOrderPlaced = false;
        currentOrderDetails = "No active order";
        currentState = WAIT_FOR_ORDER;
      }
      break;
  }
}

// ================= AUDIO CALLBACK =================
void audio_eof_mp3(const char *info) {
  Serial.print("Finished playing: "); 
  Serial.println(info);
  
  if (strstr(info, "welcome.mp3")) {
    delay(1000);
    playVoice("/enjoyfood.mp3");
  }
}

void audio_info(const char *info) {}

// ================= WEB SERVER HANDLERS =================
void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleOrder() {
  int pizza = server.arg("pizza").toInt();
  int burger = server.arg("burger").toInt();
  int donut = server.arg("donut").toInt();

  currentOrderDetails = "";
  if (pizza > 0) currentOrderDetails += String(pizza) + "x Pizza ";
  if (burger > 0) currentOrderDetails += String(burger) + "x Burger ";
  if (donut > 0) currentOrderDetails += String(donut) + "x Donut";

  isOrderPlaced = true;
  currentState = WAIT_FOR_FOOD;
  server.send(200, "text/plain", "Order Placed Successfully!");
}

void handlePickup() {
  if (currentState == WAIT_FOR_ORDER) {
    currentOrderDetails = "Plate Pickup Request";
    currentState = PICKUP_FORWARD;
    server.send(200, "text/plain", "Pickup Request Received! Robot is heading to the table.");
  } else {
    server.send(200, "text/plain", "Robot is currently busy with another task!");
  }
}

void handleStatus() {
  String statusMsg;
  if (currentState == WAIT_FOR_ORDER) statusMsg = "Ready for orders (Station)";
  else if (currentState == WAIT_FOR_FOOD) statusMsg = "Waiting for Kitchen to load food...";
  else if (currentState == DELIVER_FORWARD) statusMsg = "Food is being delivered...";
  else if (currentState == WAIT_AT_TABLE) statusMsg = "Arrived at table! Please pick up food.";
  else if (currentState == RETURN_BACKWARD) statusMsg = "Returning to station...";
  else if (currentState == PICKUP_FORWARD) statusMsg = "Going to table to collect plates...";
  else if (currentState == WAIT_AT_TABLE_PLATE) statusMsg = "Pickup arrived! Please place plate on tray.";
  else if (currentState == PICKUP_RETURN) statusMsg = "Returning to station with plates...";
  else if (currentState == WAIT_AT_STATION_PLATE) statusMsg = "Arrived at station! Please remove plate.";

  String jsonResponse = "{\"robotState\":\"" + statusMsg + "\",\"orderDetails\":\"" + currentOrderDetails + "\"}";
  server.send(200, "application/json", jsonResponse);
}

// ================= OBSTACLE & LINE SENSOR LOGIC =================
bool isObstacleDetected() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return false;

  float distanceCm = (duration * 0.0343) / 2.0;
  return (distanceCm > 0 && distanceCm <= OBSTACLE_DISTANCE_CM);
}

void followLineForward() {
  bool frontLeft  = (digitalRead(FRONT_LEFT_IR_PIN) == LINE_DETECTED);
  bool frontRight = (digitalRead(FRONT_RIGHT_IR_PIN) == LINE_DETECTED);

  if (frontLeft && !frontRight) turnLeftForward();
  else if (!frontLeft && frontRight) turnRightForward();
  else moveForward();
}

void followLineBackward() {
  bool backLeft  = (digitalRead(BACK_LEFT_IR_PIN) == LINE_DETECTED);
  bool backRight = (digitalRead(BACK_RIGHT_IR_PIN) == LINE_DETECTED);

  if (backLeft && backRight) {
    // Handled in state machine
    return;
  }

  if (backLeft && !backRight) turnLeftBackward();
  else if (!backLeft && backRight) turnRightBackward();
  else moveBackward();
}

// ================= MOTOR MOVEMENT FUNCTIONS =================
void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, BASE_SPEED); analogWrite(ENB, BASE_SPEED);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, BASE_SPEED); analogWrite(ENB, BASE_SPEED);
}

void turnLeftForward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void turnRightForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void turnLeftBackward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void turnRightBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}
