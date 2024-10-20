#define PWM_FREQUENCY 5000    // Define PWM frequency in Hz
#define PWM_MAX_VALUE 1023   // Define maximum PWM duty cycle value (10-bit resolution)
#define DELAY_TIME 100       // Delay time in milliseconds for motor speed change

// Define motor control pins based on the schematic
const int motor1Pin = D1;  // GPIO pin controlling motor 1
const int motor2Pin = D2;  // GPIO pin controlling motor 2
const int motor3Pin = D3;  // GPIO pin controlling motor 3
const int motor4Pin = D4;  // GPIO pin controlling motor 4

// Motor speed control variables (PWM duty cycle)
int motorSpeed1 = 0;
int motorSpeed2 = 0;
int motorSpeed3 = 0;
int motorSpeed4 = 0;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  
  // Set motor pins as output
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  pinMode(motor3Pin, OUTPUT);
  pinMode(motor4Pin, OUTPUT);

  // Set PWM frequency for ESP-12F (default is 1 kHz, can be adjusted if needed)
  analogWriteFreq(PWM_FREQUENCY); // Set PWM frequency to defined value
}

void loop() {
  // Increase motor speeds gradually
  for (int speed = 0; speed <= PWM_MAX_VALUE; speed++) { // Use full 10-bit resolution (0 to PWM_MAX_VALUE)
    motorSpeed1 = speed; // Set motor 1 speed
    motorSpeed2 = speed; // Set motor 2 speed
    motorSpeed3 = speed; // Set motor 3 speed
    motorSpeed4 = speed; // Set motor 4 speed

    // Apply PWM to motors using analogWrite (compatible with ESP-12F)
    analogWrite(motor1Pin, motorSpeed1); // Write PWM duty cycle to motor 1
    analogWrite(motor2Pin, motorSpeed2); // Write PWM duty cycle to motor 2
    analogWrite(motor3Pin, motorSpeed3); // Write PWM duty cycle to motor 3
    analogWrite(motor4Pin, motorSpeed4); // Write PWM duty cycle to motor 4

    // Debugging - print motor speeds to serial monitor
    Serial.print("Motor speeds: ");
    Serial.print(motorSpeed1);
    Serial.print(", ");
    Serial.print(motorSpeed2);
    Serial.print(", ");
    Serial.print(motorSpeed3);
    Serial.print(", ");
    Serial.println(motorSpeed4);

    delay(DELAY_TIME); // Delay to observe motor changes
  }

  // Decrease motor speeds gradually
  for (int speed = PWM_MAX_VALUE; speed >= 0; speed--) { // Use full 10-bit resolution (PWM_MAX_VALUE to 0)
    motorSpeed1 = speed; // Set motor 1 speed
    motorSpeed2 = speed; // Set motor 2 speed
    motorSpeed3 = speed; // Set motor 3 speed
    motorSpeed4 = speed; // Set motor 4 speed

    // Apply PWM to motors using analogWrite (compatible with ESP-12F)
    analogWrite(motor1Pin, motorSpeed1); // Write PWM duty cycle to motor 1
    analogWrite(motor2Pin, motorSpeed2); // Write PWM duty cycle to motor 2
    analogWrite(motor3Pin, motorSpeed3); // Write PWM duty cycle to motor 3
    analogWrite(motor4Pin, motorSpeed4); // Write PWM duty cycle to motor 4

    // Debugging - print motor speeds to serial monitor
    Serial.print("Motor speeds: ");
    Serial.print(motorSpeed1);
    Serial.print(", ");
    Serial.print(motorSpeed2);
    Serial.print(", ");
    Serial.print(motorSpeed3);
    Serial.print(", ");
    Serial.println(motorSpeed4);

    delay(DELAY_TIME); // Delay to observe motor changes
  }
}
