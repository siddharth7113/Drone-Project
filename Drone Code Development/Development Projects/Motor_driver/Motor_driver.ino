#define BIN1 D5 // GPIO pin connected to BIN1 on TB6612FNG
#define BIN2 D6 // GPIO pin connected to BIN2 on TB6612FNG
#define PWMB D7 // GPIO pin connected to PWMB on TB6612FNG (optional if needed)

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  Serial.println("Starting motor test...");

  // Set up motor control pins
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Start the motor in one direction and keep it running
  Serial.println("Keeping motor running in one direction...");
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 255); // Full speed

  // Motor will continue running as long as the program is running
}

void loop() {
  // No loop actions needed; motor will keep running
  Serial.println("Motor is running...");
  delay(1000); // Print a message every second to confirm motor is running
}