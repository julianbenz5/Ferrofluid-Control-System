// Define the pin where the relay is connected
const int relayPin = 6;

void setup() {
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);
}

void loop() {
  // Turn the relay ON (HIGH is typically ON for most relays)
  digitalWrite(relayPin, HIGH);
  
  // Wait for 1 second (1000 milliseconds)
  delay(1000);
  
  // Turn the relay OFF (LOW is typically OFF for most relays)
  digitalWrite(relayPin, LOW);
  
  // Wait for 1 second
  delay(0);
}
