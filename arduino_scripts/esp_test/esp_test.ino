// Blinking LED Test for ESP32
int LED_BUILTIN = 3;
void setup() {
  // Set the LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Turn the LED on (HIGH)
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Wait for a second
  delay(1000);
  
  // Turn the LED off (LOW)
  digitalWrite(LED_BUILTIN, LOW);
  
  // Wait for a second
  delay(1000);
}