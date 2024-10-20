#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>

// Wi-Fi credentials
const char* ssid = "Drone_WiFi";
const char* password = "12345678";

// MPU-6050 variables
float RateRoll = 0, RatePitch = 0, RateYaw = 0;

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// HTML content to be served from the root URL
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Drone Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
  function fetchData() {
    fetch('/mpu').then(response => response.json()).then(data => {
      document.getElementById('roll').innerText = data.roll.toFixed(2);
      document.getElementById('pitch').innerText = data.pitch.toFixed(2);
      document.getElementById('yaw').innerText = data.yaw.toFixed(2);
    });
  }

  function sendMotorCommand() {
    var speed = document.getElementById('speed').value;
    fetch(`/motor?speed=${speed}`).then(response => response.text()).then(data => {
      document.getElementById('status').innerText = data;
    });
  }

  window.onload = function () {
    fetchData();
    setInterval(fetchData, 1000); // Update MPU data every second
  };
  </script>
</head>
<body>
  <h1>Drone Control</h1>
  <h2>MPU-6050 Data</h2>
  <p>Roll: <span id="roll">0</span> °/s</p>
  <p>Pitch: <span id="pitch">0</span> °/s</p>
  <p>Yaw: <span id="yaw">0</span> °/s</p>
  
  <h2>Motor Control</h2>
  <p>Speed: <input type="range" id="speed" min="0" max="255" value="0" onchange="sendMotorCommand()"></p>
  <p>Status: <span id="status">Awaiting command...</span></p>
</body>
</html>
)rawliteral";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize I2C communication
  Wire.begin(D2, D1); // SDA, SCL for Wemos D1 Mini
  Serial.println("I2C Communication Initialized");

  // Initialize MPU-6050
  initializeMPU6050();

  // Set up Wi-Fi access point
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Route for getting MPU data
  server.on("/mpu", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getMPUData();
    request->send(200, "application/json", json);
  });

  // Route for sending motor commands
  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("speed")) {
      int speed = request->getParam("speed")->value().toInt();
      Serial.println("Motor Speed Set to: " + String(speed));
      // Implement motor speed control logic here
      request->send(200, "text/plain", "Speed set to " + String(speed));
    } else {
      request->send(400, "text/plain", "Missing speed parameter");
    }
  });

  // Start the server
  server.begin();
  Serial.println("Server Started");
}

void loop() {
  // Continuously update MPU-6050 data
  updateMPUData();

  // Print the latest MPU-6050 data
  Serial.print("Roll Rate [°/s] = ");
  Serial.print(RateRoll);
  Serial.print("\tPitch Rate [°/s] = ");
  Serial.print(RatePitch);
  Serial.print("\tYaw Rate [°/s] = ");
  Serial.println(RateYaw);

  delay(1000);  // Delay to make output readable
}

// Function to initialize MPU-6050
void initializeMPU6050() {
  Wire.beginTransmission(0x68); // Start communication with MPU-6050
  Wire.write(0x6B); // Access power management register
  Wire.write(0x00); // Wake up MPU-6050
  if (Wire.endTransmission() == 0) {
    Serial.println("MPU-6050 Wake Up Successful");
  } else {
    Serial.println("Error: Failed to Wake Up MPU-6050");
  }
  
  Wire.beginTransmission(0x68); // Start communication with MPU-6050
  Wire.write(0x1B); // Access gyroscope configuration register
  Wire.write(0x08); // Set sensitivity scale factor (±500°/s)
  if (Wire.endTransmission() == 0) {
    Serial.println("MPU-6050 Gyro Configuration Successful");
  } else {
    Serial.println("Error: Failed to Configure Gyro");
  }
}

// Function to update MPU-6050 data
void updateMPUData() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43); // Starting register for gyroscope data
  if (Wire.endTransmission() != 0) {
    Serial.println("Error: Failed to Start Data Transmission from MPU-6050");
    return;
  }

  Wire.requestFrom(0x68, 6);
  if (Wire.available() == 6) {
    int16_t GyroX = Wire.read() << 8 | Wire.read();
    int16_t GyroY = Wire.read() << 8 | Wire.read();
    int16_t GyroZ = Wire.read() << 8 | Wire.read();

    RateRoll = (float)GyroX / 65.5;
    RatePitch = (float)GyroY / 65.5;
    RateYaw = (float)GyroZ / 65.5;
  } else {
    Serial.println("Error: Failed to Read Data from MPU-6050");
  }
}

// Function to get MPU-6050 data in JSON format
String getMPUData() {
  String json = "{";
  json += "\"roll\":" + String(RateRoll) + ",";
  json += "\"pitch\":" + String(RatePitch) + ",";
  json += "\"yaw\":" + String(RateYaw);
  json += "}";
  return json;
}
