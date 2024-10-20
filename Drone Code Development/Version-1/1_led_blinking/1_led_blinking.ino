 
 /* ------------------------------------------- 
  This is to blink red and green led at interval of 4 secs */
 
 void setup() {
  pinMode(2, OUTPUT); // Internal LED of ESP32
  digitalWrite(2, HIGH);

  pinMode(5, OUTPUT); // External RED led
  digitalWrite(5, HIGH);

  delay(4000);

  
  digitalWrite(5, LOW);
  pinMode(6, OUTPUT); // Green LED of ESP32
  digitalWrite(6, HIGH);

}

void loop() {
 

}
