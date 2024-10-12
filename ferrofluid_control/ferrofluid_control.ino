// AVERAGE values are what the values will be centered on
// SD values control roughly how fast the values will change
// MR values control how strongly the values are pulled back toward AVERAGE
#define AVERAGE_ACTIVE       0.6   // what fraction of the time is the motor active (at all)
#define INACTIVE_THRESHOLD   0.2   // what speed proportion must the motor already be at to turn off
#define AVERAGE_SPEED_PROP   0.5   // what is the average proportion of the maximum speed that the motor is running at
                                   // (this is a bit hacky, it turns the relay off on and off fairly quickly to simulate speed)
                                   // (if this is not desirable behaviour, set to 1, and SD_SPEED_PROP to 0, and INACTIVE_THRESHOLD to 1)
#define SD_SPEED_PROP        0.35
#define MR_SPEED_PROP        0.4
#define AVERAGE_TIME         1     // average time until next CHANGE
#define SD_TIME              -0.4
#define MR_TIME              0.2

#define PROXIMITY_RANDOMNESS 1     // change this to add more randomness when someone comes into proximity range
                                   // keep this value fairly low; probably in the range 1-1.3
                                   // 1 is no randomness added

#define MIN_MOTOR_CHANGE_MS  10   // how often the motor can be turned on/off in ms
#define DISTANCE_THRESHOLD   50.0  // distance that triggers the proximity sensor

#define RELAYS               4
#define RELAY_PINS           {3, 4, 5, 6}
#define PROXIMITY_PIN        A0

/*
 * ----- CODE ------
 */


float random_float() {
  return (float)random(1, 10001)/10000.0;
}

// Uses the Box-Muller transform
float random_z_float() {
  float u1 = random_float();
  float u2 = random_float();
  float z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
  return z0;
}

bool motor_is_on[RELAYS] = {false};
int relay_pins[RELAYS] = RELAY_PINS;

void motor_state(bool state, int i) {
  if (state != motor_is_on[i]) {
    if (state) {
      // ON
      digitalWrite(relay_pins[i], HIGH);
    } else {
      // OFF
      digitalWrite(relay_pins[i], LOW);
    }
    motor_is_on[i] = state;
  }
}

float proximity_closeness() {
  // Read the analog value from the sensor
  int sensor_value = analogRead(PROXIMITY_PIN);

  // Convert the analog reading (0-1023) to a voltage (0-5V)
  float voltage = sensor_value * (5.0 / 1023.0);

  // Convert the voltage to distance (in cm)
  float distance_cm = (voltage / 0.0098) * 2.54;

  return distance_cm / DISTANCE_THRESHOLD;
}

// Movement vector
struct mov {
  bool active;
  float proportion;
  float ttl;
};

// All values follow the Ornstein-Uhlenbeck process
// i.e. stochiastic motion following a normal distribution

float oruhprocess(float prev, float theta, float mu, float deltat, float sigma) {
  float closeness = proximity_closeness();
  sigma *= PROXIMITY_RANDOMNESS * closeness;
  mu += 0.75 * sigma * closeness;
  return prev + theta * (mu - prev) * deltat + sigma * sqrt(deltat) * random_z_float();
}

struct mov hop_vec(struct mov vec) {
  if (vec.proportion < INACTIVE_THRESHOLD) {
    vec.active = random_float() < AVERAGE_ACTIVE;
  } else {
    vec.active = true;
  }

  vec.proportion = oruhprocess(vec.proportion, MR_SPEED_PROP, AVERAGE_SPEED_PROP, vec.ttl, SD_SPEED_PROP);
  vec.proportion = constrain(vec.proportion, 0.0, 1.0);
  vec.ttl = oruhprocess(vec.ttl, MR_TIME, AVERAGE_TIME, vec.ttl, SD_TIME);
  vec.ttl = max(vec.ttl, (float)MIN_MOTOR_CHANGE_MS / 1000.0);
  return vec;
}

struct mov default_gvec = {true, AVERAGE_SPEED_PROP, AVERAGE_TIME};
struct mov gvecs[RELAYS] = {default_gvec};

unsigned long starts[RELAYS] = {0};
unsigned long last_changes[RELAYS] = {0};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < RELAYS; i++) {
    pinMode(relay_pins[i], OUTPUT);
    digitalWrite(relay_pins[i], LOW);
    gvecs[i] = default_gvec;
  }
  randomSeed(analogRead(0));
}

void loop() {
  for (int i = 0; i < RELAYS; i++) {
    // if TTL has expired
    if ((millis() - starts[i]) > gvecs[i].ttl * 1000) {
      // get some new values
      gvecs[i] = hop_vec(gvecs[i]);
      Serial.print("VEC[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(gvecs[i].active);
      Serial.print(" ");
      Serial.print(gvecs[i].proportion);
      Serial.print(" ");
      Serial.print(gvecs[i].ttl);
      Serial.print("\n");
      Serial.print(distance_cm);
      last_changes[i] = 0;
      starts[i] = millis();
    } else {
      // If it is time to do something new to the motor
      if ((millis() - last_changes[i]) > MIN_MOTOR_CHANGE_MS) {
        if (gvecs[i].active && random_float() < gvecs[i].proportion) {
          motor_state(true, i);
        } else {
          motor_state(false, i);
        }
        last_changes[i] = millis();
      }
    }
  }
  delay(5);
}