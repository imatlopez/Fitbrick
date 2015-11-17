#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

/* Define peripherals */
Adafruit_MMA8451 mma = Adafruit_MMA8451();

/* Define constants */
#define TIMEOUT 30
#define RATE 10

/* Define variables */
float threshold = 75;
int steps = 0;

/* Boundaries */
float mx = 0; float mn = 500;

void setup(void) {
  Serial.begin(9600);

  /* Initialize sensor */
  if (!mma.begin()) {
    Serial.println("Error");
    while (1);
  }

  /* Set parameters */
  mma.setRange(MMA8451_RANGE_4_G);

  /* Initialize output as table */
  Serial.println("X,Y,Z");
}

long frame; int frames = 0;
void loop() {
  frame = millis(); frames++;
  
  /* Read from the sensor */
  mma.read();
  sensors_event_t event;
  mma.getEvent(&event);

  /* Obtain my value */
  float A = getParameter(event.acceleration.x, event.acceleration.y, event.acceleration.z);
  steps += isStep(A);

  /* Adapth threshold */
  if (A > mx) {
    mx = A;
  }
  if (A < mn) {
    mn = A;
  }
  if (frames % RATE == 0 ) {
    threshold = (mx + mn)/2;
    mx = 0; mn = 500;
  }

  /* Output */
  serialOut(event.acceleration.x, event.acceleration.y, event.acceleration.z, A);
  
  /* Timeout */
  if (millis() > TIMEOUT*1000) {
    while (1);
  } 
  else {
    delay(1000/RATE - (millis() - frame));
  }
}

float oldA = 0;
bool isStep(float A) {
  bool flag;
  if (oldA > A && A < threshold) {
    flag = 1;
  }
  else {
    flag = 0;
  }
  oldA = A;
  return flag;
}

float getParameter(float x, float y, float z) {
  return x*x + y*y + z*z;
}

void serialOut(float x, float y, float z, float a) {
  Serial.print(x); Serial.print(",");
  Serial.print(y); Serial.print(",");
  Serial.print(z); Serial.print(",");
  Serial.print(a); Serial.print(",");
  Serial.print(threshold); Serial.println();
}

