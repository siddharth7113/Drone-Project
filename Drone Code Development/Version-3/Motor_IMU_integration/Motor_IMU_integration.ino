#include <Wire.h> // Include the Wire library for I2C communication

#define MPU9250_IMU_ADDRESS 0x68 // MPU-9250 I2C address
#define GYRO_FULL_SCALE_1000_DPS 0x10 // Gyroscope full scale range (1000 dps)
#define ACC_FULL_SCALE_2G 0x00 // Accelerometer full scale range (2G)
#define LED_PIN D5           // LED pin for indicating calibration status
#define MOTOR1_PIN D1        // GPIO pin controlling motor 1
#define MOTOR2_PIN D2        // GPIO pin controlling motor 2
#define MOTOR3_PIN D3        // GPIO pin controlling motor 3
#define MOTOR4_PIN D4        // GPIO pin controlling motor 4
#define PWM_MAX_VALUE 1023   // Define maximum PWM duty cycle value (10-bit resolution)
#define DELAY_TIME 100       // Delay time in milliseconds for motor speed change

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
void controlMotors(int speed);
void printSensorData();

void setup() {
  Wire.begin(); // Initialize I2C communication
  Wire.setClock(400000); // Set I2C clock rate to 400kHz for better performance
  Serial.begin(115200); // Start serial communication for debugging with a baud rate of 115200
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output for indicating calibration status

  // Set motor pins as output
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  pinMode(MOTOR4_PIN, OUTPUT);

  configureMPU9250(); // Configure MPU-9250 settings
  calibrateSensors(); // Perform calibration
}

void loop() {
  // Increase motor speeds gradually
  for (int speed = 0; speed <= PWM_MAX_VALUE; speed++) {
    controlMotors(speed); // Set motor speeds
    readAccelerometer(); // Read accelerometer data
    readGyroscope(); // Read gyroscope data
    printSensorData(); // Print sensor data
    delay(DELAY_TIME); // Delay for readability and to observe motor changes
  }

  // Decrease motor speeds gradually
  for (int speed = PWM_MAX_VALUE; speed >= 0; speed--) {
    controlMotors(speed); // Set motor speeds
    readAccelerometer(); // Read accelerometer data
    readGyroscope(); // Read gyroscope data
    printSensorData(); // Print sensor data
    delay(DELAY_TIME); // Delay for readability and to observe motor changes
  }
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

// Function to control motor speeds
void controlMotors(int speed) {
  analogWrite(MOTOR1_PIN, speed);
  analogWrite(MOTOR2_PIN, speed);
  analogWrite(MOTOR3_PIN, speed);
  analogWrite(MOTOR4_PIN, speed);
}

// Function to print sensor data
void printSensorData() {
  // Print accelerometer data to the serial monitor
  Serial.print("ACCEL: ");
  Serial.print("X: "); Serial.print(accelerometer.x); // Print X-axis accelerometer value
  Serial.print(" Y: "); Serial.print(accelerometer.y); // Print Y-axis accelerometer value
  Serial.print(" Z: "); Serial.println(accelerometer.z); // Print Z-axis accelerometer value

 
  // Print gyroscope data to the serial monitor
  Serial.print("GYRO: ");
  Serial.print("X: "); Serial.print(gyroscope.x); // Print X-axis gyroscope value
  Serial.print(" Y: "); Serial.print(gyroscope.y); // Print Y-axis gyroscope value
  Serial.print(" Z: "); Serial.println(gyroscope.z); // Print Z-axis gyroscope value
}
