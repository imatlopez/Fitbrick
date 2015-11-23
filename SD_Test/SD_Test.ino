#include <SPI.h>      // SD Card
#include <SD.h>

/* Define Constants */
#define SDRATE 30000
#define RATE 100

/* Define variables */
int activity = 1;
long hiddenSteps;
long localSteps;
long globalSteps;

/* Define timer */
long lastStep, saveData, frameStart;

void setup() {

  Serial.begin(9600);

  /* Set SD Card */
  if (!SD.begin()) {
    Serial.println(F("Error: SD Failed"));
    while (1);
  }

  /* Restore Values */
  restore();
}

void loop() {
  frameStart = millis();

  /* Calculate activity level */
  int spm = int(float(hiddenSteps) / ( float(millis() - saveData) / 60000.0 ));
  if (spm < 120 && activity != 1) {
    activity = 1;
    store();
  }
  else if (spm >= 160 && activity != 3) {
    activity = 3;
    store();
  }
  else if (spm >= 120 && spm < 160 && activity != 2) {
    activity = 2;
    store();
  }

  /* Store values */
  if (millis() - saveData > SDRATE) {
    saveData = millis();
    store();
  }

  /* Timeout */
  long safety = RATE - (millis() - frameStart);
  if (safety < 0) {
    safety = 0;
  }
  delay(safety);
}

void store() {
  Serial.println("Chikin");
  /* Update */
  globalSteps += hiddenSteps;
  localSteps += hiddenSteps;

  /* Save */
  File root = SD.open(F("ftbrklog.csv"), FILE_WRITE);
  String output = String(globalSteps);
  output += F(",");
  output += String(hiddenSteps);
  output += F(",");
  output += String(activity);
  output += F(",");
  output += String(millis()/1000);
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
    root.println("Steps,Delta,Activity,Secs");
    root.close();
  }
}

