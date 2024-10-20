#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// Wi-Fi credentials
const char* ssid = "Drone_WiFi";
const char* password = "12345678";

// Pin definitions
const int LED_PIN = D4;           // Single LED for indication
const int SER_PIN = D5;           // Serial data input (shift register)
const int SRCLK_PIN = D6;         // Shift register clock
const int RCLK_PIN = D7;          // Storage register clock
const int PWMA = D8;               //PWMA pin always to be on high

// MPU-9250 variables
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
int CalibrationCount = 5000;
bool calibrated = false;

// Shift register control for motors
byte motorControl = 0b00000000;  // Initialize all motors to be off

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// HTML for the web interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Drone Control Interface</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
    }
    button {
      padding: 10px 20px;
      margin: 10px;
      font-size: 18px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <h1>Drone Control Interface</h1>
  <button onclick="startDrone()">Start Drone</button>
  <button onclick="stopDrone()">Stop Drone</button>
  <p>Status: <span id="status">Idle</span></p>
  <p>MPU-9250 Readings:</p>
  <p>Roll: <span id="roll">0</span> °/s</p>
  <p>Pitch: <span id="pitch">0</span> °/s</p>
  <p>Yaw: <span id="yaw">0</span> °/s</p>

  <script>
    function startDrone() {
      fetch('/start').then(response => response.text()).then(data => {
        document.getElementById('status').innerText = data;
      });
    }

    function stopDrone() {
      fetch('/stop').then(response => response.text()).then(data => {
        document.getElementById('status').innerText = data;
      });
    }

    function updateReadings() {
      fetch('/readings').then(response => response.json()).then(data => {
        document.getElementById('roll').innerText = data.roll;
        document.getElementById('pitch').innerText = data.pitch;
        document.getElementById('yaw').innerText = data.yaw;
      });
    }

    setInterval(updateReadings, 1000);
  </script>
</body>
</html>
)rawliteral";

// Function to read and process gyroscope data
void gyro_signals() {
    Wire.beginTransmission(0x68); // Starting I2C communication with gyro
    Wire.write(0x1A); // Switching to low pass filters
    Wire.write(0x05);
    Wire.endTransmission();

    Wire.beginTransmission(0x68); // Setting the sensitivity scale factor
    Wire.write(0x1B);
    Wire.write(0x8);
    Wire.endTransmission();

    Wire.beginTransmission(0x68);  // Accessing registers storing gyro measurements
    Wire.write(0x43);
    Wire.endTransmission();

    Wire.requestFrom(0x68, 6);

    int16_t GyroX = Wire.read() << 8 | Wire.read(); // Reads the gyro measurement around the X-axis
    int16_t GyroY = Wire.read() << 8 | Wire.read(); // Reads the gyro measurement around the Y-axis
    int16_t GyroZ = Wire.read() << 8 | Wire.read(); // Reads the gyro measurement around the Z-axis

    RateRoll = (float)GyroX / 65.5;
    RatePitch = (float)GyroY / 65.5;
    RateYaw = (float)GyroZ / 65.5;
}

// Function to update the shift register with motor control data
void updateShiftRegister() {
  digitalWrite(RCLK_PIN, LOW); // Prepare for data shifting
  shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, motorControl); // Send data to the shift register
  digitalWrite(RCLK_PIN, HIGH); // Latch the data to outputs
}

void startDrone() {
  // Two motors go forward, two go backward
  motorControl = 0b00010110;  // Example: Motor A forward, Motor B backward
  updateShiftRegister();
  digitalWrite(LED_PIN, HIGH);  // LED on
}

void stopDrone() {
  motorControl = 0b00000000;  // Stop motors
  updateShiftRegister();
  digitalWrite(LED_PIN, LOW);  // LED off
}

void setup() {
    Serial.begin(115200);

    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize shift register pins
    pinMode(SER_PIN, OUTPUT);
    pinMode(SRCLK_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);

    // Initialize I2C communication for the MPU-6050
    Wire.begin();
    Wire.setClock(400000); // I2C clock speed

    // Wake up the MPU-6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission() == 0) {
        Serial.println("MPU-6050 connected");
    } else {
        Serial.println("Error connecting to MPU-6050");
    }

    // Perform 2-second calibration for MPU-6050
    Serial.println("Calibrating MPU-6050...");
    for (RateCalibrationNumber = 0; RateCalibrationNumber < CalibrationCount ; RateCalibrationNumber++) {
        gyro_signals();
        RateCalibrationRoll += RateRoll;
        RateCalibrationPitch += RatePitch;
        RateCalibrationYaw += RateYaw;
        delay(1);  // Short delay
    }

    // Calculate the average calibration values
    RateCalibrationRoll /= CalibrationCount;
    RateCalibrationPitch /= CalibrationCount;
    RateCalibrationYaw /= CalibrationCount;
    Serial.println("Calibration complete");

    // Initialize Wi-Fi Access Point
    WiFi.softAP(ssid, password);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Serve the embedded HTML code
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });

    // Handle start request
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
      startDrone();
      request->send(200, "text/plain", "Drone Started");
    });

    // Handle stop request
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
      stopDrone();
      request->send(200, "text/plain", "Drone Stopped");
    });

    // Handle readings request
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
      gyro_signals();
      RateRoll -= RateCalibrationRoll;
      RatePitch -= RateCalibrationPitch;
      RateYaw -= RateCalibrationYaw;

      String json = "{\"roll\":" + String(RateRoll, 2) +
                    ",\"pitch\":" + String(RatePitch, 2) +
                    ",\"yaw\":" + String(RateYaw, 2) + "}";
      request->send(200, "application/json", json);
    });

    // Start server
    server.begin();
}

void loop() {
    // Keep the LED on while the drone is running
    digitalWrite(PWMA, HIGH); // keeping PWM pin high for motor to keep running
    digitalWrite(LED_PIN, motorControl != 0 ? HIGH : LOW);
}
