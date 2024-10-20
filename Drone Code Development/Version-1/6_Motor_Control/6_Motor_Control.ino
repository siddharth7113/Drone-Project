#define GatePin D7 // Correct GPIO pin for D7 on WeMos D1 mini

void setup() {
  pinMode(GatePin, OUTPUT);
  Serial.begin(9600);

  // Adjust PWM frequency
  // analogWriteFreq(2000); // Set PWM frequency to 2 kHz
  analogWriteFreq(1000);  // Set PWM frequency to 1 kHz
  // analogWriteFreq(5000);  // Set PWM frequency to 5 kHz

}

void loop() {
  // Automatic control example
  for (int motorSpeed = 0; motorSpeed <= 255; motorSpeed++) {
    analogWrite(GatePin, motorSpeed);
    Serial.println(motorSpeed);
    delay(500); // Wait for 20 milliseconds
  }

  for (int motorSpeed = 255; motorSpeed >= 0; motorSpeed--) {
    analogWrite(GatePin, motorSpeed);
    Serial.println(motorSpeed);
    delay(500); // Wait for 20 milliseconds
  }
}

// #define GatePin D7 // Correct GPIO pin for D7 on WeMos D1 mini

// void setup() {
//   pinMode(GatePin, OUTPUT);
//   Serial.begin(9600);

//   // Adjust PWM frequency
//   // analogWriteFreq(2000); // Set PWM frequency to 2 kHz
//   analogWriteFreq(1000);  // Set PWM frequency to 1 kHz
//   // analogWriteFreq(5000);  // Set PWM frequency to 5 kHz

//   // Set the duty cycle to 100%
//   analogWrite(GatePin, 255);
// }

// void loop() {
//   // No need to adjust the duty cycle, it remains at 100%
// }
