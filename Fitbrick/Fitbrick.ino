/******************************************************************/
/*
      @file: Fitbrick
      @authors: Matias Lopez, Raiyan Sobhan

*/
/******************************************************************/

/* Loading Libraries */
#include <Wire.h>     // I2C
#include <SPI.h>      // SD Card
#include <SD.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

/* Initialize Peripherals */
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_MMA8451      mma = Adafruit_MMA8451();
sensors_event_t event;
int x, y, z;

/* Define Constants */
#define SDRATE 30000
#define BRATE 300
#define TRATE 500
#define KRATE 20000
#define RATE 100
#define TSTEP 500
#define RSTEP 3
#define WEIGHT 10
#define SLEEP 10000
#define SON 0x7
#define SOFF 0x0
#define BDEB 150
#define BUZZ 3

/* Define variables */
int activity = 1;
int threshold;
unsigned long hiddenSteps;
unsigned long localSteps;
unsigned long globalSteps;
int userAct;

/* Define debounces */
bool screenID;
bool screenOn;

/* Define timer */
long lastStep, screenTime, frameStart, saveData, buttonDebounce, lastBuzz, kickAss, lazyAss;


void setup(void) {
  Serial.begin(9600);

  /* Initialize LCD */
  lcd.begin(16, 2);
  enableScreen(1);
  lcd.print(F("  Initializing                  "));

  /* Set SD Card */
  if (!SD.begin()) {
    Serial.println(F("Error: SD Failed"));
    while (1);
  }

  /* Initialize sensor */
  if (!mma.begin()) {
    Serial.println(F("Error: MMA Failed"));
    while (1);
  }

  /* Set parameters and default */
  mma.setRange(MMA8451_RANGE_4_G);

  /* Restore Values */
  restore();
}

unsigned long frames;
int sx, sy, sz, ox, oy, oz;
void loop() {
  frames++; frameStart = millis();

  /* Accept user input */
  uint8_t buttons = lcd.readButtons();
  if (buttons && millis() - buttonDebounce > BDEB) {
    enableScreen(1); buttonDebounce = millis();
    if (buttons & BUTTON_LEFT || buttons & BUTTON_RIGHT) {
      if (screenID == 0) {
        screenID = 1;
      }
      else if (screenID == 1) {
        screenID = 0;
      }
    }
    else if (buttons & BUTTON_SELECT) {
      localSteps = 0;
    }
    else if ((buttons & BUTTON_UP) && userAct < 3) {
      userAct++;
    }
    else if ((buttons & BUTTON_DOWN) && userAct > 0) {
      userAct--;
    }
  }

  /* Read from the sensor */
  mma.read(); mma.getEvent(&event);
  x = 100 * event.acceleration.x;
  y = 100 * event.acceleration.y;
  z = 100 * event.acceleration.z;

  /* Set orientation baseline */
  if (frames % BRATE == 1) {
    ox = (ox + x) / 2; oy = (oy + y) / 2; oz = (oz + z) / 2;
  }

  /* Calculate activity level */
  int spm = int(float(hiddenSteps) / ( float(millis() - saveData) / 60000.0 ));
  if (spm < 120 && activity != 1) {
    activity = 1;
    store(spm);
  }
  else if (spm >= 160 && activity != 3) {
    activity = 3;
    if (millis() - lazyAss > KRATE) {
      kickAss = millis();
    }
    store(spm);
  }
  else if (spm >= 120 && spm < 160 && activity != 2) {
    activity = 2;
    store(spm);
  }
  if (activity == 3) {
    lazyAss = millis();
  }

  /* Obtain my value */
  int param = getParameter();
  hiddenSteps += 2 * isStep(param);

  /* Refresh screen */
  if (millis() - screenTime < SLEEP) {
    if (screenOn == 0) {
      enableScreen(1);
    }
    if (frames % 2 == 0) {
      screenInfo();
    }
  }
  else if (screenOn == 1) {
    enableScreen(0);
  }

  /* Store values */
  if (millis() - saveData > SDRATE) {
    saveData = millis();
    store(spm);
  }

  /* Adapt threshold */
  threshold = ((WEIGHT - 1) * threshold + param) / WEIGHT;
  if (threshold < 10) {
    threshold = 10;
  }

  /* Trainer Mode */
  if (userAct > activity) {
    if (millis() - lastBuzz > TRATE) {
      tone(BUZZ, 4100, RATE);
      lastBuzz = millis();
    }
  }
  else {
    noTone(BUZZ);
  }

  /* Timeout */
  long safety = RATE - (millis() - frameStart);
  if (safety < 0) {
    safety = 0;
  }
  delay(safety);
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
    return int(RSTEP) * int(flag);
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
  return (op[0] + op[1] + op[2] + op[3]) / 4;
}

void screenInfo() {

  /* First line */
  lcd.setCursor(0, 0);
  if (userAct > 0) {
    lcd.print(F(" Training Mode "));
    lcd.print(String(userAct));
  }
  else if (screenID == 1) {
    String X = String(x);
    String Y = String(y);
    String Z = String(z);

    for (int i = 0; i < 5 - X.length(); i++) {
      lcd.print(F(" "));
    }
    lcd.print(X);
    for (int i = 0; i < 5 - Y.length(); i++) {
      lcd.print(F(" "));
    }
    lcd.print(Y);
    for (int i = 0; i < 5 - Z.length(); i++) {
      lcd.print(F(" "));
    }
    lcd.print(Z);
    lcd.print(F(" "));
  }
  else {
    String local = String(localSteps); String kickButt;
    if (millis() - lazyAss < KRATE) {
      kickButt = String( (millis() - kickAss) / 1000 );
    }
    lcd.print(local);
    for (int i = 0; i < 16 - local.length() - kickButt.length(); i++) {
      lcd.print(F(" "));
    }
    lcd.print(kickButt);
  }

  /* Second Line */
  lcd.setCursor(0, 1);
  String act; String global = String(globalSteps);
  for (int i = 0; i < activity; i++) {
    act += F("#");
  }
  lcd.print(global);
  for (int i = 0; i < 16 - global.length() - act.length(); i++) {
    lcd.print(F(" "));
  }
  lcd.print(act);
}

void enableScreen(bool state) {
  if (state == 1) {
    screenOn = 1;
    lcd.setBacklight(SON);
    screenTime = millis();
  }
  else {
    screenOn = 0;
    lcd.clear();
    lcd.setBacklight(SOFF);
  }
}



void store(int s) {
  /* Update */
  globalSteps += hiddenSteps;
  localSteps += hiddenSteps;

  /* Save */
  File root = SD.open(F("ftbrklog.csv"), FILE_WRITE);
  String output = String(globalSteps);
  output += F(",");
  output += String(hiddenSteps);
  output += F(",");
  output += String(s);
  output += F(",");
  output += String(activity);
  output += F(",");
  output += String(millis() / 1000);
  root.println(output);
  root.close();

  /* Reset */
  hiddenSteps = 0;
}

void restore() {
  if (SD.exists(F("ftbrklog.csv"))) {
    /* Open File */
    File root = SD.open(F("ftbrklog.csv"), FILE_READ);
    String memory; int state; bool header = 1;

    /* Read through file */
    while (root.available()) {
      char c = root.read();
      if (c == '\n') {
        state = 1;
      }
      else if (c == ',') {
        state = 2;
      }
      else if (state != 2) {
        if (state == 1) {
          memory = F("");
          header = 0;
          state = 0;
        }
        memory += c;
      }
    }

    /* Finalize */
    if (header == 0) {
      globalSteps = memory.toInt();
    }
    root.close();
  }
  else {
    File root = SD.open(F("ftbrklog.csv"), FILE_WRITE);
    root.println("Steps,Delta,SPM,Activity,Secs");
    root.close();
  }
}
