#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

/* Define peripherals */
Adafruit_MMA8451 mma = Adafruit_MMA8451();
sensors_event_t event;
int x, y, z;

/* Define constants */
#define TIMEOUT 60000
#define BRATE 300
#define RATE 100
#define TSTEP 500
#define RSTEP 4
#define WEIGHT 10

/* Define variables */
int threshold = 0;
int steps = 0;
int param;

/* Define timers */
long lastStep, frameStart;

/* Define parameter variables */
int frames = 0;
int ox, oy, oz, sx, sy, sz;

void setup(void) {
  Serial.begin(9600);

  /* Initialize sensor */
  if (!mma.begin()) {
    Serial.println("Error");
    while (1);
  }

  /* Set parameters and default */
  mma.setRange(MMA8451_RANGE_4_G);

  /* Initialize output as csv compliant */
  Serial.println("X,Y,Z,A,T,S");
}

void loop() {
  frames++; frameStart = millis();
  
  /* Read from the sensor */
  mma.read(); mma.getEvent(&event);
  x = 100*event.acceleration.x;
  y = 100*event.acceleration.y;
  z = 100*event.acceleration.z;

  /* Set orientation baseline */
  if (frames % BRATE == 1) {
    ox = (ox + x)/2; oy = (oy + y)/2; oz = (oz + z)/2;
  }

  /* Obtain my value */
  int param = getParameter();
  steps += 2*isStep(param);

  /* Adapt threshold */
  threshold = ((WEIGHT - 1) * threshold + param) / WEIGHT;

  /* Output */
  serialOut(param);
  
  /* Timeout */
  if (millis() > long(TIMEOUT)) {
    while (1);
  } 
  else {
    delay(RATE - (millis() - frameStart));
  }
}

int oldParam = 0; int stepsSinceReset;
int isStep(int param) {
  int flag;

  /* Detect step */
  if (param <= threshold && oldParam > threshold) {
    flag = 1;
    /* Verify Integrity of Step */
    if ((millis() - lastStep) < long(TSTEP)) {
      /* Reset thresholding */
      sx = 0; sy = 0; sz = 0;
      ox = x; oy = y; oz = z;
      stepsSinceReset = 0;
      threshold = 0;
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
  if (stepsSinceReset < int(RSTEP)) {
    return 0;
  }
  else if (stepsSinceReset == int(RSTEP)) {
    return int(RSTEP)*int(flag);
  }
  else {
    return flag;
  }
}

int op[] = {0, 0, 0, 0};
int getParameter() {
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

void serialOut(int a) {
  Serial.print(x); Serial.print(",");
  Serial.print(y); Serial.print(",");
  Serial.print(z); Serial.print(",");
  Serial.print(a); Serial.print(",");
  Serial.print(threshold); Serial.print(",");
  Serial.print(steps); Serial.println();
}

