float Voltage;

int R1 = 1100;
int R2 = 10000;

// Function to read battery voltage
void battery_voltage() {
    float sensorValue = analogRead(D0);
    // Serial.println(sensorValue);
    Voltage = (sensorValue / 1023.0) * 3.3 * (R1 + R2) / R2; // Adjust based on your resistor values
    // Voltage = sensorValue/(3.3*11);
    Serial.println("V");
    Serial.print(Voltage);
}

void setup() {
 Serial.begin(57600);
 pinMode(D0,OUTPUT);
//  pinMode(13,INPUT);
 digitalWrite(D0,HIGH);

}

void loop() {
  battery_voltage();
 
  Serial.println("V");
  delay(50);
  delay(1000);
}
