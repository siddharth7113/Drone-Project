#define MotorPin1 D3 // GPIO pin for the first motor
#define MotorPin2 D4 // GPIO pin for the second motor
#define MotorPin3 D7 // GPIO pin for the third motor
#define MotorPin4 D8 // GPIO pin for the fourth motor

void setup() {
  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  pinMode(MotorPin3, OUTPUT);
  pinMode(MotorPin4, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Automatic control example
  for (int motorSpeed = 0; motorSpeed <= 255; motorSpeed++) {
    analogWrite(MotorPin1, motorSpeed);
    analogWrite(MotorPin2, motorSpeed);
    analogWrite(MotorPin3, motorSpeed);
    analogWrite(MotorPin4, motorSpeed);
    Serial.println(motorSpeed);
    delay(20); // Wait for 20 milliseconds
  }

  for (int motorSpeed = 255; motorSpeed >= 0; motorSpeed--) {
    analogWrite(MotorPin1, motorSpeed);
    analogWrite(MotorPin2, motorSpeed);
    analogWrite(MotorPin3, motorSpeed);
    analogWrite(MotorPin4, motorSpeed);
    Serial.println(motorSpeed);
    delay(20); // Wait for 20 milliseconds
  }
}
