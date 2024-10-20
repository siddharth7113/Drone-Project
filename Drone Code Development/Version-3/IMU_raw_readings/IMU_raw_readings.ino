#include <Wire.h> // Include the Wire library for I2C communication

#define MPU9250_IMU_ADDRESS 0x68 // MPU-9250 I2C address
#define GYRO_FULL_SCALE_1000_DPS 0x10 // Gyroscope full scale range (1000 dps)
#define ACC_FULL_SCALE_2G 0x00 // Accelerometer full scale range (2G)
#define GYRO_SENSITIVITY_1000DPS 65.5 // Sensitivity for 1000 dps

// Raw data structures for sensor readings
struct gyroscope_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the gyroscope
} gyroscope;

struct accelerometer_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the accelerometer
} accelerometer;

struct magnetometer_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the magnetometer
} magnetometer;

struct temperature_raw {
  int16_t value; // Temperature reading
} temperature;

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

void setup() {
  Wire.begin(); // Initialize I2C communication
  Wire.setClock(400000); // Set I2C clock rate to 400kHz for better performance
  Serial.begin(115200); // Start serial communication for debugging with a baud rate of 115200

  // Configure MPU-9250 gyro and accelerometer ranges
  I2CwriteByte(MPU9250_IMU_ADDRESS, 27, GYRO_FULL_SCALE_1000_DPS); // Set gyroscope range to 1000 dps
  I2CwriteByte(MPU9250_IMU_ADDRESS, 28, ACC_FULL_SCALE_2G); // Set accelerometer range to 2G
}

void loop() {
  // Read accelerometer data
  uint8_t rawData[6]; // Array to hold raw accelerometer data (6 bytes)
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x3B, 6, rawData); // Read 6 bytes starting from ACCEL_XOUT_H (accelerometer data)
  accelerometer.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  accelerometer.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  accelerometer.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis

  // Read gyroscope data
  I2CreadBytes(MPU9250_IMU_ADDRESS, 0x43, 6, rawData); // Read 6 bytes starting from GYRO_XOUT_H (gyroscope data)
  gyroscope.x = (int16_t)((rawData[0] << 8) | rawData[1]); // Combine high and low bytes for X-axis
  gyroscope.y = (int16_t)((rawData[2] << 8) | rawData[3]); // Combine high and low bytes for Y-axis
  gyroscope.z = (int16_t)((rawData[4] << 8) | rawData[5]); // Combine high and low bytes for Z-axis

  // Convert gyroscope raw data to degrees per second
  float rateRoll = gyroscope.x / GYRO_SENSITIVITY_1000DPS;
  float ratePitch = gyroscope.y / GYRO_SENSITIVITY_1000DPS;
  float rateYaw = gyroscope.z / GYRO_SENSITIVITY_1000DPS;

  // Print accelerometer data to the serial monitor
  Serial.print("ACCEL: ");
  Serial.print("X: "); Serial.print(accelerometer.x); // Print X-axis accelerometer value
  Serial.print(" Y: "); Serial.print(accelerometer.y); // Print Y-axis accelerometer value
  Serial.print(" Z: "); Serial.println(accelerometer.z); // Print Z-axis accelerometer value

  // Print gyroscope data to the serial monitor
  Serial.print("GYRO: ");
  Serial.print("Roll: "); Serial.print(rateRoll); // Print roll rate in degrees per second
  Serial.print(" Pitch: "); Serial.print(ratePitch); // Print pitch rate in degrees per second
  Serial.print(" Yaw: "); Serial.println(rateYaw); // Print yaw rate in degrees per second

  delay(500); // Delay for readability between data readings
}
