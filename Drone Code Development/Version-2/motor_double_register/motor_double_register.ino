const int SER_PIN = D6;    // Serial data input (shift register)
const int SRCLK_PIN = D7;  // Shift register clock
const int RCLK_PIN = D8;   // Storage register clock

// Motor control bits for all four motors (AIN1, AIN2, BIN1, BIN2, CIN1, CIN2, DIN1, DIN2)
byte motorControl = 0b00000000;  // Initialize all motors to be off

void setup() {
  pinMode(SER_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  
  // Initialize motor states
  updateShiftRegister();
}

void loop() {
  // Example: Set Motor A, B, C, and D to move forward at full speed (AIN1=1, AIN2=0; BIN1=1, BIN2=0; etc.)
  motorControl = 0b01010101;  // AIN1 high, AIN2 low; BIN1 high, BIN2 low; CIN1 high, CIN2 low; DIN1 high, DIN2 low
  updateShiftRegister();
  delay(1000);

  // Example: Set all motors to reverse direction (AIN1=0, AIN2=1; BIN1=0, BIN2=1; etc.)
  motorControl = 0b10101010;  // AIN1 low, AIN2 high; BIN1 low, BIN2 high; CIN1 low, CIN2 high; DIN1 low, DIN2 high
  updateShiftRegister();
  delay(1000);
}

void updateShiftRegister() {
  digitalWrite(RCLK_PIN, LOW); // Prepare for data shifting
  shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, motorControl); // Send data to the shift register
  digitalWrite(RCLK_PIN, HIGH); // Latch the data to outputs
}
