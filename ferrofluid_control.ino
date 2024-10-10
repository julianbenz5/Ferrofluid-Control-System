// Define the analog pin connected to the sensor
const int sensorPin = A0; // Pin where the AN pin of the MaxSonar is connected
const int relayPin = 3;   // Pin connected to the relay

// Variables to store the sensor reading and calculated distance
int sensorValue = 0;
float distanceCm = 0.0;

// Distance threshold for turning the relay on/off
const float distanceThreshold = 50.0; // 50 cm

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Set the relay pin as an output
  pinMode(relayPin, OUTPUT);

  // Make sure the relay is off initially
  digitalWrite(relayPin, LOW);
}

void loop() {
  // Read the analog value from the sensor
  sensorValue = analogRead(sensorPin);

  // Convert the analog reading (0-1023) to a voltage (0-5V)
  float voltage = sensorValue * (5.0 / 1023.0);

  // Convert the voltage to distance (in cm)
  distanceCm = (voltage / 0.0098) * 2.54;

  // Plot the distance in centimeters on the Serial Plotter
  Serial.println(distanceCm);

  // Check if the distance is below the threshold
  if (distanceCm < distanceThreshold) {
    // Turn the relay on if the distance is below the threshold
    digitalWrite(relayPin, HIGH);
  } else {
    // Turn the relay off if the distance is above the threshold
    digitalWrite(relayPin, LOW);
  }

  // Small delay to allow for sensor stability
  delay(50);
}
