// Pin assignment
const int ledPin = D7;  // LED control pin

void setup() {
  pinMode(ledPin, OUTPUT); // Initialize the LED pin as an output
  Serial.begin(115200);     // Start serial communication for debugging
}

void loop() {
  // Increase LED brightness
  for (int brightness = 0; brightness <= 255; brightness += 10) {
    analogWrite(ledPin, brightness);  // Write the brightness to LED
    Serial.println("Increasing Brightness: " + String(brightness));
    delay(500);  // Wait for 100 milliseconds
  }

  // Decrease LED brightness
  for (int brightness = 255; brightness >= 0; brightness -= 10) {
    analogWrite(ledPin, brightness);  // Write the brightness to LED
    Serial.println("Decreasing Brightness: " + String(brightness));
    delay(500);  // Wait for 100 milliseconds
  }
  
  delay(2000); // Wait for a second before starting over
}
