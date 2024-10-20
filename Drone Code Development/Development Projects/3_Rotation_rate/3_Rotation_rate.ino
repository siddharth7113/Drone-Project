  /*---------------------------------------------------------------

Note: The standars pins for I2C communication is SDA-21 and SCL - 22 but any other pins could be used too.
For more detailed information check this :

https://chewett.co.uk/blog/2543/i2c-connection-pins-on-the-wemos-d1-mini-esp8266/
https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/

I2C Pin	Pin on the WeMos
SCL (Serial Clock)	D1
SDA (Serial Data)	D2

--------------------------------------------------------------*/

#include<Wire.h>

float RateRoll, RatePitch, RateYaw;

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

void setup() {
  Serial.begin(57600);
  pinMode(2,OUTPUT);  // Internal LED
  digitalWrite(2,HIGH);

  Wire.setClock(400000); // Setting I2C clock speed
  Wire.begin(); // Need to define GPIO pins for serial communication
  delay(250);


  Wire.beginTransmission(0x68);   // Starting the gyro in the power mode
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
}

void loop() {
  
  gyro_signals();
  Serial.print("Roll Rate [°/s] = ");
  Serial.print(RateRoll);
  Serial.print("\t");
  Serial.print("\t");
  Serial.print("Pitch Rate [°/s] = ");
  Serial.print(RatePitch);
  Serial.print("\t");
  Serial.print("Yaw Rate [°/s] = ");
  Serial.print(RateYaw);
  Serial.println("");
  delay(50);

}
