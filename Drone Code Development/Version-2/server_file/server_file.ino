#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// Wi-Fi Credentials
const char* ssid = "Drone_WiFi";
const char* password = "12345678";

// Pin Definitions
const int RED_LED_PIN = D3;
const int GREEN_LED_PIN = D4;
const int BATTERY_PIN = D0;
const int MPU_SDA = D2;
const int MPU_SCL = D1;



// MPU-6050 Variables
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int R1 = 1100;
int R2 = 10000;
float batteryVoltage = 0.0;

// Create Web Server on Port 80
AsyncWebServer server(80);

// Embed the HTML code as a string
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
        background-color: #f0f0f0;
        text-align: center;
    }

    button {
        padding: 10px 20px;
        margin: 10px;
        font-size: 16px;
        cursor: pointer;
    }

    .joystick-container {
        display: flex;
        justify-content: center;
        align-items: center;
        flex-wrap: wrap;
        margin: 20px;
    }

    .joystick {
        width: 150px;
        height: 150px;
        background-color: #00aaff;
        border-radius: 50%;
        position: relative;
        margin: 20px;
        display: inline-block;
    }

    .inner-circle {
        width: 50px;
        height: 50px;
        background-color: #fff;
        border-radius: 50%;
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
    }

    .status {
        margin-top: 20px;
    }

    @media screen and (max-width: 768px) {
        .joystick-container {
            flex-direction: column;
        }
    }

    @media screen and (min-width: 769px) {
        .joystick-container {
            flex-direction: row;
        }
    }
  </style>
</head>
<body>
  <h1>Drone Control Interface</h1>
  <button id="startButton">Start Drone</button>
  <button id="calibrateButton">Calibrate Drone</button>
  <div class="joystick-container">
    <div class="joystick" id="moveJoystick"><div class="inner-circle"></div></div>
    <p>Movement Control</p>
  </div>
  <div class="joystick-container">
    <div class="joystick" id="altitudeJoystick"><div class="inner-circle"></div></div>
    <p>Altitude/Yaw Control</p>
  </div>
  <div class="status">
    <p>Status: <span id="status">Waiting...</span></p>
    <p>Battery Level: <span id="batteryLevel">0%</span></p>
  </div>
  <script>
    document.addEventListener("DOMContentLoaded", function () {
      const startButton = document.getElementById('startButton');
      const calibrateButton = document.getElementById('calibrateButton');
      const statusElement = document.getElementById('status');
      const batteryLevel = document.getElementById('batteryLevel');

      startButton.addEventListener('click', function () {
        console.log('Start Drone');
        statusElement.innerText = 'Drone Started';
      });

      calibrateButton.addEventListener('click', function () {
        console.log('Calibrate Drone');
        statusElement.innerText = 'Calibrating...';
        setTimeout(() => {
          statusElement.innerText = 'Calibration Successful';
        }, 2000);
      });

      setInterval(() => {
        fetch('/status')
          .then(response => response.json())
          .then(data => {
            batteryLevel.innerText = data.battery + "%";
            statusElement.innerText = "Roll: " + data.roll.toFixed(2) + ", Pitch: " + data.pitch.toFixed(2) + ", Yaw: " + data.yaw.toFixed(2);
          });
      }, 5000);

      const moveJoystick = document.getElementById('moveJoystick');
      moveJoystick.addEventListener('mousemove', function (event) {
        console.log('Movement Joystick Moved:', event.offsetX, event.offsetY);
      });

      const altitudeJoystick = document.getElementById('altitudeJoystick');
      altitudeJoystick.addEventListener('mousemove', function (event) {
        console.log('Altitude Joystick Moved:', event.offsetX, event.offsetY);
      });
    });
  </script>
</body>
</html>
)rawliteral";

// Function to read battery voltage
void battery_voltage() {
    int sensorValue = analogRead(BATTERY_PIN);
    batteryVoltage = (sensorValue / 1023.0) * 3.7 * (R1 + R2) / R2;
    Serial.print("Battery Voltage: ");
    Serial.print(batteryVoltage);
    Serial.println("V");
}

// Function to handle MPU-6050 gyroscope data
void gyro_signals() {
    Wire.beginTransmission(0x68); // Start I2C communication
    Wire.write(0x1A); // Access low pass filters
    Wire.write(0x05);
    Wire.endTransmission();

    Wire.beginTransmission(0x68); // Set sensitivity scale factor
    Wire.write(0x1B);
    Wire.write(0x08);
    Wire.endTransmission();

    Wire.beginTransmission(0x68); // Access registers storing gyroscope measurements
    Wire.write(0x43);
    Wire.endTransmission();

    Wire.requestFrom(0x68, 6);

    int16_t GyroX = Wire.read() << 8 | Wire.read();
    int16_t GyroY = Wire.read() << 8 | Wire.read();
    int16_t GyroZ = Wire.read() << 8 | Wire.read();

    RateRoll = (float)GyroX / 65.5;
    RatePitch = (float)GyroY / 65.5;
    RateYaw = (float)GyroZ / 65.5;

    Serial.print("Roll Rate: ");
    Serial.print(RateRoll);
    Serial.print("\tPitch Rate: ");
    Serial.print(RatePitch);
    Serial.print("\tYaw Rate: ");
    Serial.println(RateYaw);
}

// Setup function
void setup() {
    Serial.begin(115200);

    // Initialize LEDs
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Initialize MPU-6050
    Wire.begin(MPU_SDA, MPU_SCL);
    Wire.beginTransmission(0x68); // Start MPU in power mode
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    // Initialize Wi-Fi Access Point
    WiFi.softAP(ssid, password);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // API endpoint to return battery voltage and gyroscope data
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String response = "{\"battery\":" + String(batteryVoltage, 2) + ",\"roll\":" + String(RateRoll) +
                          ",\"pitch\":" + String(RatePitch) + ",\"yaw\":" + String(RateYaw) + "}";
        request->send(200, "application/json", response);
    });

    // Start server
    server.begin();
}

// Main loop
void loop() {
    // Read battery voltage
    battery_voltage();

    // Read MPU-6050 gyroscope data
    gyro_signals();

    delay(5000); // Adjust the delay as needed
}
