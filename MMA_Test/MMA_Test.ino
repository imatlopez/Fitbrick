#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

/* Define peripherals */
Adafruit_MMA8451 mma = Adafruit_MMA8451();

/* Define constants */
#define TIMEOUT 60000
#define RATE 100
#define TSTEP 500
#define RSTEP 4

/* Define variables */
int threshold = 0;
int steps = 0;

/* Define timers */
long lastStep, frameStart, thresholdReset;

/* Define parameter variables */
int ox, oy, oz, sx, sy, sz, op[4];

void setup(void) {
  Serial.begin(9600);

  /* Initialize sensor */
  if (!mma.begin()) {
    Serial.println("Error");
    while (1);
  }

  /* Set parameters */
  mma.setRange(MMA8451_RANGE_4_G);

  /* Initialize output as csv compliant */
  Serial.println("X,Y,Z,A,T,S");
}

void loop() {
  frameStart = millis();
  
  /* Read from the sensor */
  mma.read();
  sensors_event_t event;
  mma.getEvent(&event);

  /* Obtain my value */
  int param = getParameter(100*event.acceleration.x, 100*event.acceleration.y, 100*event.acceleration.z);
  steps += 2*isStep(param);

  /* Adapt threshold */
  threshold *= (millis() - RATE);
  threshold /= 1000;
  threshold += param/10;
  threshold /= millis();
  threshold *= 1000;

  /* Output */
  serialOut(event.acceleration.x, event.acceleration.y, event.acceleration.z, param);
  
  /* Timeout */
  if (millis() > TIMEOUT) {
    while (1);
  } 
  else {
    delay(RATE - (millis() - frameStart));
  }
}

int oldParam = 0; int stepsSinceReset;
bool isStep(int param) {
  bool flag;

  /* Detect step */
  if (param < threshold && oldParam > threshold) {
    flag = 1;
    /* Verify Integrity of Step */
    if (millis() - lastStep < TSTEP) {
      /* Reset thresholding */
      sx = 0; sy = 0; sz = 0;
      stepsSinceReset = 0;
      thresholdReset = millis();
    }
    else {
      stepsSinceReset++;
    }
    lastStep = millis();
  }
  else {
    flag = 0;
  }
  oldParam = param;

  /* Determine if data is reliable to publish */
  if (stepsSinceReset < RSTEP) {
    return 0;
  }
  else if (stepsSinceReset == RSTEP) {
    return RSTEP*flag;
  }
  else {
    return flag;
  }
}

int getParameter(int x, int y, int z) {
  /* Cycle through moving averager */
  op[0] = op[1]; op[1] = op[2]; op[2] = op[3];

  /* Update sum parameter */
  sx += x - ox; sy += y - oy; sz += z - oz;

  /* Update old values */
  ox = x; oy = y; oz = z;

  /* Determine parameter */
  op[3] = sy;
  if (op[3] < sx) {
    op[3] = sx;
  }
  if (op[3] < sz) {
    op[3] = sz;
  }

  /* Return moving average */
  return (op[0] + op[1] + op[2] + op[3])/4;
}

void serialOut(float x, float y, float z, float a) {
  Serial.print(x); Serial.print(",");
  Serial.print(y); Serial.print(",");
  Serial.print(z); Serial.print(",");
  Serial.print(a); Serial.print(",");
  Serial.print(threshold); Serial.print(",");
  Serial.print(steps); Serial.println();
}

