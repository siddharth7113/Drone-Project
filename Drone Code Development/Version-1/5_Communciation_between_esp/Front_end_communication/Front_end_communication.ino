// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>

// Set your access point network credentials
const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

// Define motor control pins
const int motorPin = D1;  // GPIO5 on Wemos D1 mini

// MPU variables
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// HTML content to be served from the root URL
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP8266 MPU Control</title>
  <script>
  function fetchData() {
    fetch('/update').then(response => response.json()).then(data => {
      document.getElementById('roll').innerText = data.roll.toFixed(2);
      document.getElementById('pitch').innerText = data.pitch.toFixed(2);
      document.getElementById('yaw').innerText = data.yaw.toFixed(2);
      setTimeout(fetchData, 100); // update every second
    });
  }
  function sendMotorCommand() {
    var speed = document.getElementById('speed').value;
    fetch(`/motor?speed=${speed}`);
  }
  window.onload = function () {
    fetchData(); // Start fetching the data
  };
  </script>
</head>
<body>
  <h1>MPU Readings</h1>
  <p>Roll: <span id="roll">0</span> degrees/s</p>
  <p>Pitch: <span id="pitch">0</span> degrees/s</p>
  <p>Yaw: <span id="yaw">0</span> degrees/s</p>
  <h2>Motor Control</h2>
  <input type="range" id="speed" min="0" max="1023">
  <button onclick="sendMotorCommand()">Set Speed</button>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // I2C clock speed
  pinMode(motorPin, OUTPUT); // Set motor pin as output
  analogWriteFreq(1000); // Set PWM frequency for motor control

  
  Wire.beginTransmission(0x68);   // Starting the gyro in the power mode
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

    // Performing the calibration step
  for(RateCalibrationNumber=0; RateCalibrationNumber < 2000; RateCalibrationNumber++){
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);

  }
  
  // getting the calibrated values
  RateCalibrationRoll /= 2000; 
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web server endpoints
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"roll\":" + String(RateRoll) + ",\"pitch\":" + String(RatePitch) + ",\"yaw\":" + String(RateYaw) + "}";
    request->send(200, "application/json", json);
    Serial.println("{\"roll\":" + String(RateRoll) + ",\"pitch\":" + String(RatePitch) + ",\"yaw\":" + String(RateYaw) + "}");  
  });

  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("speed")) {
      int speed = request->getParam("speed")->value().toInt();
      analogWrite(motorPin, speed);
      request->send(200, "text/plain", "Motor speed set to " + String(speed));
    } else {
      request->send(400, "text/plain", "Speed parameter missing");
    }
  });

  server.begin();
}

void loop() {
  gyro_signals();
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;

  delay(50);
}

void gyro_signals(void){

  Wire.beginTransmission(0x68); // Starting I2C communication with gyro
  Wire.write(0x1A); // Switching to low pass filters
  Wire.write(0x05);
  Wire.endTransmission();

  Wire.beginTransmission(0x68); // Setting the senstivity scale factor
  Wire.write(0x1B);
  Wire.write(0x8);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);  // Accessing registers storing gyro measurments
  Wire.write(0x43);
  Wire.endTransmission();

  Wire.requestFrom(0x68,6);

  int16_t GyroX = Wire.read()<<8 | Wire.read(); // Reads the gyro measurmemnt around the X-axis
  int16_t GyroY = Wire.read()<<8 | Wire.read(); // Reads the gyro measurmemnt around the Y-axis
  int16_t GyroZ = Wire.read()<<8 | Wire.read(); // Reads the gyro measurmemnt around the Z-axis

  RateRoll = (float)GyroX/65.5;
  RatePitch = (float)GyroY/65.5;
  RateYaw = (float)GyroZ/65.5;

}
