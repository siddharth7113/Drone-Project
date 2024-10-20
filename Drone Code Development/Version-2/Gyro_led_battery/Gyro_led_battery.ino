#include <Wire.h>

// Pin definitions (revised)
const int RED_LED_PIN = D4;  // New GPIO pin for Red LED
const int BLUE_LED_PIN = D3;  // New GPIO pin for Green LED
const int BATTERY_PIN = D0; // New GPIO pin for battery voltage reading


// Battery monitoring variables
float batteryVoltage = 0.0;

// MPU-6050 variables
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
int CalibrationCount = 5000;

int R1 = 1100;
int R2 = 10000;

// Function to read battery voltage
void battery_voltage() {
    int sensorValue = analogRead(BATTERY_PIN);
    batteryVoltage = (sensorValue / 1023.0) * 3.3 * (R1 + R2) / R2; // Adjust based on your resistor values
}

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

void setup() {
    // Initialize serial communication
    Serial.begin(57600);

    // Initialize LEDs
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(BATTERY_PIN, OUTPUT);

    // Turn on the red LED to indicate calibration mode
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BATTERY_PIN, HIGH);
    Serial.println("Red LED is ON: Entering calibration mode");

    // Initialize I2C communication for the MPU-6050
    Wire.begin();
    Wire.setClock(400000); // I2C clock speed

    // Wake up the MPU-6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    // Short delay to ensure MPU-6050 is ready
    delay(100);

    // Initial readings before calibration for debugging
    Serial.println("Initial MPU-6050 Readings:");
    gyro_signals();
    Serial.print("Initial Roll Rate [°/s] = ");
    Serial.print(RateRoll);
    Serial.print("\tInitial Pitch Rate [°/s] = ");
    Serial.print(RatePitch);
    Serial.print("\tInitial Yaw Rate [°/s] = ");
    Serial.println(RateYaw);

    // Calibration process
    for (RateCalibrationNumber = 0; RateCalibrationNumber < CalibrationCount ; RateCalibrationNumber++) {
        gyro_signals();
        RateCalibrationRoll += RateRoll;
        RateCalibrationPitch += RatePitch;
        RateCalibrationYaw += RateYaw;
        delay(1);
    }

    // Calculate the average calibration values
    RateCalibrationRoll /= CalibrationCount;
    RateCalibrationPitch /= CalibrationCount;
    RateCalibrationYaw /= CalibrationCount;

    // Indicate that calibration is complete by turning off the red LED and turning on the green LED
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println("Red LED is OFF: Calibration complete");
    
    digitalWrite(BLUE_LED_PIN, HIGH);
    Serial.println("Green LED is ON: System is ready");
}

void loop() {
    // Ensure the green LED stays on in the loop
    digitalWrite(BLUE_LED_PIN, HIGH);

    // Read the battery voltage
    battery_voltage();
    Serial.print("Battery Voltage: ");
    Serial.print(batteryVoltage);
    Serial.println("V");

    // Read and calibrate gyroscope data
    gyro_signals();
    RateRoll -= RateCalibrationRoll;
    RatePitch -= RateCalibrationPitch;
    RateYaw -= RateCalibrationYaw;

    Serial.print("Roll Rate [°/s] = ");
    Serial.print(RateRoll);
    Serial.print("\tPitch Rate [°/s] = ");
    Serial.print(RatePitch);
    Serial.print("\tYaw Rate [°/s] = ");
    Serial.println(RateYaw);

    // Small delay before the next loop iteration
    delay(600); // Adjust the delay as needed
}
