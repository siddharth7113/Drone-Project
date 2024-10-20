#include <Wire.h> // Include the Wire library for I2C communication
#include <ESPAsyncWebServer.h> // Include ESPAsyncWebServer library for asynchronous communication
#include <ESP8266WiFi.h> // Include ESP8266 WiFi library

#define MPU9250_IMU_ADDRESS 0x68 // MPU-9250 I2C address
#define GYRO_FULL_SCALE_1000_DPS 0x10 // Gyroscope full scale range (1000 dps)
#define ACC_FULL_SCALE_2G 0x00 // Accelerometer full scale range (2G)
#define LED_PIN D5           // LED pin for indicating calibration status

// WiFi credentials
const char* ssid = "IMU_AP_MODE";
const char* password = "12345678";

AsyncWebServer server(80); // Create an asynchronous server on port 80

// Raw data structures for sensor readings
struct gyroscope_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the gyroscope
} gyroscope;

struct accelerometer_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the accelerometer
} accelerometer;

// Offset values for calibration
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;

void I2CwriteByte(uint8_t address, uint8_t registerAddress, uint8_t data);
void I2CreadBytes(uint8_t address, uint8_t registerAddress, uint8_t count, uint8_t * dest);
void configureMPU9250();
void calibrateSensors();
void readAccelerometer();
void readGyroscope();
String getIMUData();

void setup() {
  Wire.begin(); // Initialize I2C communication
  Wire.setClock(400000); // Set I2C clock rate to 400kHz for better performance
  Serial.begin(115200); // Start serial communication for debugging with a baud rate of 115200
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output for indicating calibration status

  configureMPU9250(); // Configure MPU-9250 settings
  calibrateSensors(); // Perform calibration

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
      <title>IMU Data</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <script>
      function fetchData() {
        fetch('/imu').then(response => response.json()).then(data => {
          document.getElementById('accel').innerText = `ACCEL: X: ${data.accelX} Y: ${data.accelY} Z: ${data.accelZ}`;
          document.getElementById('gyro').innerText = `GYRO: X: ${data.gyroX} Y: ${data.gyroY} Z: ${data.gyroZ}`;
        });
      }
      window.onload = function () {
        setInterval(fetchData, 1000);
      };
      </script>
    </head>
    <body>
      <h1>IMU Data from ESP8266</h1>
      <p id="accel">ACCEL: X: 0 Y: 0 Z: 0</p>
      <p id="gyro">GYRO: X: 0 Y: 0 Z: 0</p>
    </body>
    </html>
    )rawliteral");
  });

  server.on("/imu", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getIMUData());
  });

  server.begin(); // Start the server
  Serial.println("Server Started");
}

void loop() {
  // No need for code here as AsyncWebServer handles requests asynchronously
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

// Function to calibrate sensors
void calibrateSensors() {
  unsigned long startTime = millis();
  int sampleCount = 0;
  long gyroSumX = 0, gyroSumY = 0, gyroSumZ = 0;
  long accelSumX = 0, accelSumY = 0, accelSumZ = 0;

  while (millis() - startTime < 2000) { // Calibrate for 2 seconds
    digitalWrite(LED_PIN, millis() % 500 < 250 ? HIGH : LOW); // Blink LED during calibration

    readAccelerometer();
    readGyroscope();

    // Accumulate readings for averaging
    gyroSumX += gyroscope.x;
    gyroSumY += gyroscope.y;
    gyroSumZ += gyroscope.z;
    accelSumX += accelerometer.x;
    accelSumY += accelerometer.y;
    accelSumZ += accelerometer.z;
    sampleCount++;
  }

  // Calculate offsets by averaging the accumulated readings
  gyroOffsetX = gyroSumX / (float)sampleCount;
  gyroOffsetY = gyroSumY / (float)sampleCount;
  gyroOffsetZ = gyroSumZ / (float)sampleCount;
  accelOffsetX = accelSumX / (float)sampleCount;
  accelOffsetY = accelSumY / (float)sampleCount;
  accelOffsetZ = accelSumZ / (float)sampleCount;

  // Turn LED on to indicate end of calibration
  digitalWrite(LED_PIN, HIGH);
}

// Function to read accelerometer data
void readAccelerometer() {
  uint8_t rawData[6]; // Array to hold raw accelerometer data (6 bytes)
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x3B, 6, rawData); // Read 6 bytes starting from ACCEL_XOUT_H (accelerometer data)
  accelerometer.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  accelerometer.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  accelerometer.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis

  // Adjust accelerometer readings with calibration offsets
  accelerometer.x -= accelOffsetX;
  accelerometer.y -= accelOffsetY;
  accelerometer.z -= accelOffsetZ;
}

// Function to read gyroscope data
void readGyroscope() {
  uint8_t rawData[6]; // Array to hold raw gyroscope data (6 bytes)
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x43, 6, rawData); // Read 6 bytes starting from GYRO_XOUT_H (gyroscope data)
  gyroscope.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  gyroscope.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  gyroscope.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis

  // Adjust gyroscope readings with calibration offsets
  gyroscope.x -= gyroOffsetX;
  gyroscope.y -= gyroOffsetY;
  gyroscope.z -= gyroOffsetZ;
}

// Function to get IMU data in JSON format
String getIMUData() {
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
