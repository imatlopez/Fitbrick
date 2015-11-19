/******************************************************************/
/*
 *    @file: Fitbrick
 *    @authors: Matias Lopez, Raiyan Sobhan
 *
 */
/******************************************************************/

/* Loading Libraries */
#include <Wire.h>     // I2C
#include <SPI.h>      // SD Card
#include <SD.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_RGBLCDShield.h>

/* Initialize Peripherals */
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_MMA8451      mma = Adafruit_MMA8451();
sensors_event_t event;
int x, y, z;

/* Define Constants */
#define BRATE 300
#define RATE 100
#define TSTEP 500
#define RSTEP 4
#define WEIGHT 10

/* Define variables */
int threshold = 0;
int hiddenSteps = 0;
int localSteps = 0;
int globalSteps = 0;

/* Define timer */
long lastStep, frameStart;


void setup(void) {
  Serial.begin(9600);

  /* Initialize sensor */
  if (!mma.begin()) {
    Serial.println("Error");
    while (1);
  }

  /* Set parameters and default */
  mma.setRange(MMA8451_RANGE_4_G);
}

int frames = 0;
int sx, sy, sz, ox, oy, oz;
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
  hiddenSteps += 2*isStep(param);

  /* Adapt threshold */
  threshold = ((WEIGHT - 1) * threshold + param) / WEIGHT;
  
  /* Timeout */
  delay(RATE - (millis() - frameStart));
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

