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

void gyro_signals(void) {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  
  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();
  
  RateRoll = (float)GyroX / 65.5;
  RatePitch = (float)GyroY / 65.5;
  RateYaw = (float)GyroZ / 65.5;
}

void setup(){
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // I2C clock speed
  
  pinMode(motorPin, OUTPUT); // Set motor pin as output
  analogWriteFreq(1000); // Set PWM frequency for motor control
  
  // Initialize MPU
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(250);
  
  // Calibrate MPU
  for (RateCalibrationNumber = 0; RateCalibrationNumber < 2000; RateCalibrationNumber++) {
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);
  }
  
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
  
  // Setting the ESP as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Define server routes for motor control
  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("speed")) {
      int speed = request->getParam("speed")->value().toInt();
      analogWrite(motorPin, speed);
      request->send(200, "text/plain", "Motor speed set to " + String(speed));
    } else {
      request->send(400, "text/plain", "Speed parameter missing");
    }
  });

  // Start server
  server.begin();
}
 
void loop(){
  gyro_signals();
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;
  // Serial.print("Roll Rate [°/s] = ");
  // Serial.print(RateRoll);
  // Serial.print("\t");
  // Serial.print("\t");
  // Serial.print("Pitch Rate [°/s] = ");
  // Serial.print(RatePitch);
  // Serial.print("\t");
  // Serial.print("Yaw Rate [°/s] = ");
  // Serial.print(RateYaw);
  // Serial.println("");
  delay(50);
}
