const int SER_PIN = D6;    // Serial data input (shift register)
const int SRCLK_PIN = D7;  // Shift register clock
const int RCLK_PIN = D8;   // Storage register clock

byte motorControl = 0b00000000;  // Initialize all motors to be off

void setup() {
  pinMode(SER_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  
  // Initialize motor states
  updateShiftRegister();
}

void loop() {
  // Example: Set Motor A to move forward at full speed
  motorControl = 0b00000101;  // AIN1 high, AIN2 low, PWM high for Motor A
  updateShiftRegister();
  delay(1000);

  // Example: Set Motor B to move forward at full speed
  motorControl = 0b00101000;  // BIN1 high, BIN2 low, PWM high for Motor B
  updateShiftRegister();
  delay(1000);
}

void updateShiftRegister() {
  digitalWrite(RCLK_PIN, LOW); // Prepare for data shifting
  shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, motorControl); // Send data to the shift register
  digitalWrite(RCLK_PIN, HIGH); // Latch the data to outputs
}
