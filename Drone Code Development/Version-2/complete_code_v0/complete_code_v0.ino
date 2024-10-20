#include <Wire.h>

// Pin definitions
const int LED_PIN = D3;           // Single LED for indication
const int SER_PIN = D5;           // Serial data input (shift register)
const int SRCLK_PIN = D6;         // Shift register clock
const int RCLK_PIN = D7;          // Storage register clock
const int PWMA = D8;               //PWMA pin always to be on high 

// MPU-6050 variables
float RateRoll, RatePitch, RateYaw;    

// Shift register control for motors
byte motorControl = 0b00000000;  // Initialize all motors to be off

void setup() {       
    Serial.begin(115200);

    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Ensure LED is off at startup
    pinMode(PWMA, OUTPUT);
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
}

void loop() {
    // Read and process gyroscope data
    startMotorWithBoost();
    gyro_signals();
    // Print MPU-6050 data to serial monitor
    Serial.print("Roll Rate [°/s] = ");
    Serial.print(RateRoll);
    Serial.print("\tPitch Rate [°/s] = ");
    Serial.print(RatePitch);
    Serial.print("\tYaw Rate [°/s] = ");
    Serial.println(RateYaw);

    // Toggle LED to indicate the loop is running
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    
    // Test motor control: Motor A forward, Motor B backward
    motorControl = 0b00010110;  // Example: Motor A forward, Motor B backward
    updateShiftRegister();

    delay(1000);  // Delay for 1 second
}

void startMotorWithBoost() {
    analogWrite(PWMA, 100);  // Provide an initial boost
    delay(200);              // Wait for 200ms to start the motor
    analogWrite(PWMA, 50);  // Reduce to a safe running level
}

// Function to read and process gyroscope data
void gyro_signals() {
    Wire.beginTransmission(0x68); // Starting I2C communication with gyro
    Wire.write(0x43);  // Accessing registers storing gyro measurements
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
