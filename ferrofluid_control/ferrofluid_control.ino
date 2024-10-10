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
#define SD_TIME              0.4
#define MR_TIME              0.2

#define PROXIMITY_RANDOMNESS 1     // change this to add more randomness when someone comes into proximity range
                                   // keep this value fairly low; probably in the range 1-1.3
                                   // 1 is no randomness added

#define MIN_MOTOR_CHANGE_MS  100   // how often the motor can be turned on/off in ms
#define DISTANCE_THRESHOLD   50.0  // distance that triggers the proximity sensor

#define RELAY_PIN            3
#define PROXIMITY_PIN        A0

/*
 * ----- CODE ------
 */


float random_float() {
  return (float)random(10001)/10000.0;
}

float random_z_float() {
  int rg = 0;
  for (int i = 0; i < 100; i++) {
    rg += random(1001);
  }
  return ((float)rg-100000)/100000.0;
}

bool motor_is_on = false;

void motor_state(bool state) {
  if (state != motor_is_on) {
    if (state) {
      // ON
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      // OFF
      digitalWrite(RELAY_PIN, LOW);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

bool proximity_active() {
  // Read the analog value from the sensor
  int sensor_value = analogRead(PROXIMITY_PIN);

  // Convert the analog reading (0-1023) to a voltage (0-5V)
  float voltage = sensor_value * (5.0 / 1023.0);

  // Convert the voltage to distance (in cm)
  float distance_cm = (voltage / 0.0098) * 2.54;

  return distance_cm < DISTANCE_THRESHOLD;
}

// Movement vector
struct mov {
  bool active;
  float proportion;
  int ttl;
};

// All values follow the Ornstein-Uhlenbeck process
// i.e. stochiastic motion following a normal distribution

float oruhprocess(float prev, float theta, float mu, float deltat, float sigma) {
  if (proximity_active()) sigma *= PROXIMITY_RANDOMNESS;
  return prev + theta * (mu - prev) * deltat + sigma * sqrt(deltat) * random_z_float();
}

struct mov hop_vec(struct mov vec) {
  if (vec.proportion < INACTIVE_THRESHOLD) {
    vec.active = random_float() < AVERAGE_ACTIVE;
  } else {
    vec.active = true;
  }

  vec.proportion = oruhprocess(vec.proportion, MR_SPEED_PROP, AVERAGE_SPEED_PROP, vec.ttl, SD_SPEED_PROP);
  vec.ttl = oruhprocess(vec.ttl, MR_TIME, AVERAGE_TIME, vec.ttl, SD_TIME);
  return vec;
}

struct mov gvec = {true, AVERAGE_SPEED_PROP, AVERAGE_TIME};

void loop() {
  gvec = hop_vec(gvec);
  char buf[128];
  sprintf(buf, "VEC: %d %f %f\n", gvec.active, gvec.proportion, gvec.ttl);
  Serial.print(buf);

  unsigned long start = millis();
  unsigned long last_change = 0;
  while ((millis() - start) < gvec.ttl * 1000) {
    if ((millis() - last_change) > MIN_MOTOR_CHANGE_MS) {
      if (gvec.active && random_float() < gvec.proportion) {
        motor_state(true);
      } else {
        motor_state(false);
      }
      last_change = millis();
    }
    delay(10);
  }
}
