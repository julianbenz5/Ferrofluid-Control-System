// Define the pins where the relays are connected
const int relayPin1 = 3;
const int relayPin2 = 4;
const int relayPin3 = 5;
const int relayPin4 = 6;

void setup() {
  // Initialize the relay pins as outputs
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  
  // Turn all relays ON
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);
  digitalWrite(relayPin3, HIGH);
  digitalWrite(relayPin4, HIGH);
}

void loop() {
  // No action needed in the loop as relays stay ON
}
