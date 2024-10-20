#include <ESPAsyncWebServer.h> // Include ESPAsyncWebServer library for asynchronous communication
#include <ESP8266WiFi.h> // Include ESP8266 WiFi library
#include <Wire.h> // Include Wire library for I2C communication

#define LED_PIN D5           // LED pin for indicating calibration status
#define MOTOR1_PIN D1        // GPIO pin controlling motor 1
#define MOTOR2_PIN D2        // GPIO pin controlling motor 2
#define MOTOR3_PIN D3        // GPIO pin controlling motor 3
#define MOTOR4_PIN D4        // GPIO pin controlling motor 4
#define PWM_MAX_VALUE 1023   // Define maximum PWM duty cycle value (10-bit resolution)
#define PWM_MIN_VALUE 0      // Define minimum PWM duty cycle value
#define MPU9250_IMU_ADDRESS 0x68 // MPU-9250 I2C address
#define GYRO_FULL_SCALE_1000_DPS 0x10 // Gyroscope full scale range (1000 dps)
#define ACC_FULL_SCALE_2G 0x00 // Accelerometer full scale range (2G)

// WiFi credentials
const char* ssid = "Motor_Control_AP";
const char* password = "12345678";

AsyncWebServer server(80); // Create an asynchronous server on port 80

int motorSpeeds[4] = {0, 0, 0, 0}; // Store the speeds for each motor

struct gyroscope_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the gyroscope
} gyroscope;

struct accelerometer_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the accelerometer
} accelerometer;

void I2CwriteByte(uint8_t address, uint8_t registerAddress, uint8_t data);
void I2CreadBytes(uint8_t address, uint8_t registerAddress, uint8_t count, uint8_t * dest);
void configureMPU9250();
void readAccelerometer();
void readGyroscope();
String getIMUData();

void setup() {
  Serial.begin(115200); // Start serial communication for debugging with a baud rate of 115200
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output for indicating status

  // Set motor pins as output
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  pinMode(MOTOR4_PIN, OUTPUT);

  // Set up I2C communication
  Wire.begin();
  Wire.setClock(400000); // Set I2C clock rate to 400kHz for better performance
  configureMPU9250(); // Configure MPU-9250 settings

  // Set up WiFi in AP mode
  WiFi.softAP(ssid, password); // Start WiFi in AP mode
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP()); // Print the IP address

  // Set up server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", R"rawliteral(
    <!DOCTYPE HTML><html>
    <head>
      <title>Motor and IMU Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <script>
      function sendMotorCommand(motor, action) {
        fetch(`/motor?motor=${motor}&action=${action}`).then(response => response.text()).then(data => {
          document.getElementById('status').innerText = data;
        }).catch(error => {
          document.getElementById('status').innerText = 'Error: ' + error;
        });
      }

      function fetchIMUData() {
        fetch('/imu').then(response => response.json()).then(data => {
          document.getElementById('accel').innerText = `ACCEL: X: ${data.accelX} Y: ${data.accelY} Z: ${data.accelZ}`;
          document.getElementById('gyro').innerText = `GYRO: X: ${data.gyroX} Y: ${data.gyroY} Z: ${data.gyroZ}`;
        });
      }

      window.onload = function () {
        setInterval(fetchIMUData, 1000);
      };
      </script>
    </head>
    <body>
      <h1>Motor and IMU Control from ESP8266</h1>
      <div>
        <h2>Motor 1</h2>
        <button onclick="sendMotorCommand(1, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(1, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 2</h2>
        <button onclick="sendMotorCommand(2, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(2, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 3</h2>
        <button onclick="sendMotorCommand(3, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(3, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 4</h2>
        <button onclick="sendMotorCommand(4, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(4, 'down')">Speed Down</button>
      </div>
      <p>Status: <span id="status">Awaiting command...</span></p>
      <h2>IMU Data</h2>
      <p id="accel">ACCEL: X: 0 Y: 0 Z: 0</p>
      <p id="gyro">GYRO: X: 0 Y: 0 Z: 0</p>
    </body>
    </html>
    )rawliteral");
  });

  // Route for sending motor commands
  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("motor") && request->hasParam("action")) {
      int motor = request->getParam("motor")->value().toInt();
      String action = request->getParam("action")->value();

      if (motor >= 1 && motor <= 4) {
        if (action == "up") {
          motorSpeeds[motor - 1] = min(motorSpeeds[motor - 1] + 50, PWM_MAX_VALUE); // Increase speed by 50, limit to max value
        } else if (action == "down") {
          motorSpeeds[motor - 1] = max(motorSpeeds[motor - 1] - 50, PWM_MIN_VALUE); // Decrease speed by 50, limit to min value
        } else {
          request->send(400, "text/plain", "Invalid action. Use 'up' or 'down'.");
          return;
        }

        controlMotor(motor, motorSpeeds[motor - 1]);
        digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate motor control command received
        request->send(200, "text/plain", "Motor " + String(motor) + " speed set to " + String(motorSpeeds[motor - 1]));
      } else {
        request->send(400, "text/plain", "Invalid motor number. Please provide a value between 1 and 4.");
      }
    } else {
      request->send(400, "text/plain", "Missing motor or action parameter");
    }
  });

  // Route for getting IMU data
  server.on("/imu", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getIMUData());
  });

  server.begin(); // Start the server
  Serial.println("Server Started");
}

void loop() {
  // No need for code here as AsyncWebServer handles requests asynchronously
}

// Function to control individual motor speed
void controlMotor(int motor, int speed) {
  switch (motor) {
    case 1:
      analogWrite(MOTOR1_PIN, speed);
      break;
    case 2:
      analogWrite(MOTOR2_PIN, speed);
      break;
    case 3:
      analogWrite(MOTOR3_PIN, speed);
      break;
    case 4:
      analogWrite(MOTOR4_PIN, speed);
      break;
  }
  Serial.println("Motor " + String(motor) + " speed set to: " + String(speed));
  delay(10); // Small delay for stability
  digitalWrite(LED_PIN, LOW); // Turn off LED after setting motor speed
}

// Function to write a byte of data to a specific I2C register
void I2CwriteByte(uint8_t address, uint8_t registerAddress, uint8_t data) {
  Wire.beginTransmission(address); // Start communication with the specified I2C address
  Wire.write(registerAddress); // Specify the register to write to
  Wire.write(data); // Write the data to the specified register
  Wire.endTransmission(); // End the transmission
}

// Function to read a specified number of bytes from a specific I2C register
void I2CreadBytes(uint8_t address, uint8_t registerAddress, uint8_t count, uint8_t * dest) {
  Wire.beginTransmission(address); // Start communication with the specified I2C address
  Wire.write(registerAddress); // Specify the register to read from
  Wire.endTransmission(false); // End transmission but keep the connection active for reading
  Wire.requestFrom(address, count); // Request the specified number of bytes from the device
  for (uint8_t i = 0; i < count; i++) {
    dest[i] = Wire.read(); // Read each byte and store it in the destination array
  }
}

// Function to configure MPU-9250 settings
void configureMPU9250() {
  // Configure MPU-9250 gyro and accelerometer ranges
  I2CwriteByte(MPU9250_IMU_ADDRESS, 27, GYRO_FULL_SCALE_1000_DPS); // Set gyroscope range to 1000 dps
  I2CwriteByte(MPU9250_IMU_ADDRESS, 28, ACC_FULL_SCALE_2G); // Set accelerometer range to 2G
}

// Function to read accelerometer data
void readAccelerometer() {
  uint8_t rawData[6]; // Array to hold raw accelerometer data (6 bytes)
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x3B, 6, rawData); // Read 6 bytes starting from ACCEL_XOUT_H (accelerometer data)
  accelerometer.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  accelerometer.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  accelerometer.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis
}

// Function to read gyroscope data
void readGyroscope() {
  uint8_t rawData[6]; // Array to hold raw gyroscope data (6 bytes)
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x43, 6, rawData); // Read 6 bytes starting from GYRO_XOUT_H (gyroscope data)
  gyroscope.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  gyroscope.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  gyroscope.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis
}

// Function to get IMU data in JSON format
String getIMUData() {
  readAccelerometer();
  readGyroscope();
  String json = "{";
  json += "\"accelX\":" + String(accelerometer.x) + ",";
  json += "\"accelY\":" + String(accelerometer.y) + ",";
  json += "\"accelZ\":" + String(accelerometer.z) + ",";
  json += "\"gyroX\":" + String(gyroscope.x) + ",";
  json += "\"gyroY\":" + String(gyroscope.y) + ",";
  json += "\"gyroZ\":" + String(gyroscope.z);
  json += "}";
  return json;
}
