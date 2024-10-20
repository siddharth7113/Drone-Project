#include <Wire.h> // Include the Wire library for I2C communication

#define MPU9250_MAG_ADDRESS 0x0C // Magnetometer (AK8963) I2C address
#define MPU9250_IMU_ADDRESS 0x68 // MPU-9250 I2C address

// Raw data structure for magnetometer readings
struct magnetometer_raw {
  int16_t x, y, z; // X, Y, Z axis readings for the magnetometer
} magnetometer;

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

  // Enable bypass mode to access the magnetometer directly
  I2CwriteByte(MPU9250_IMU_ADDRESS, 0x37, 0x02); // Write to INT_PIN_CFG register to enable bypass mode

  // Configure the magnetometer to continuous measurement mode with 16-bit output
  I2CwriteByte(MPU9250_MAG_ADDRESS, 0x0A, 0x16); // Set magnetometer to continuous measurement mode 2 (100Hz, 16-bit)
}

void loop() {
  // Read magnetometer data
  uint8_t rawData[6]; // Array to hold raw magnetometer data (6 bytes)
  I2CreadBytes(MPU9250_MAG_ADDRESS, 0x03, 6, rawData); // Read 6 bytes starting from HXL (magnetometer data)
  magnetometer.x = (int16_t)((rawData[1] << 8) | rawData[0]); // Combine low and high bytes for X-axis
  magnetometer.y = (int16_t)((rawData[3] << 8) | rawData[2]); // Combine low and high bytes for Y-axis
  magnetometer.z = (int16_t)((rawData[5] << 8) | rawData[4]); // Combine low and high bytes for Z-axis

  // Print magnetometer data to the serial monitor
  Serial.print("MAG: ");
  Serial.print("X: "); Serial.print(magnetometer.x); // Print X-axis magnetometer value
  Serial.print(" Y: "); Serial.print(magnetometer.y); // Print Y-axis magnetometer value
  Serial.print(" Z: "); Serial.println(magnetometer.z); // Print Z-axis magnetometer value

  delay(500); // Delay for readability between data readings
}

