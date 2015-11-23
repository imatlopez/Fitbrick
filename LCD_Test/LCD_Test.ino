/* Loading Libraries */
#include <Wire.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

/* Initialize Peripherals */
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
int x, y, z;

/* Define Constants */
#define SLEEP 10000
#define BDEB 50
#define SON 0x7
#define SOFF 0x0

/* Define variables */
int activity = 2;
long localSteps = 2340;
long globalSteps = 1247865;


/* Define debounces */
bool screenID = 0;
bool screenOn = 0;

/* Define clocks */
long screenTime, frameStart, buttonDebounce;

void setup() {
  Serial.begin(9600);

  /* Initialize LCD */
  lcd.begin(16, 2);
  enableScreen(1);
  lcd.print(F("  Initializing                  "));

  /* Finalize */
  delay(2000);
}

int frames = 0;
void loop() {
  frames++; frameStart = millis();

  int param = 100;

  /* Accept user input */
  uint8_t buttons = lcd.readButtons();
  if (buttons && millis() - buttonDebounce > BDEB) {
    enableScreen(1); buttonDebounce = millis();
    if (buttons & BUTTON_LEFT || buttons & BUTTON_RIGHT) {
      if (screenID == 0) {
        screenID = 1;
      }
      else {
        screenID = 0;
      }
    }
    else if (buttons & BUTTON_SELECT) {
      localSteps = 0;
    }
  }

  /* Refresh screen */
  if (millis() - screenTime < SLEEP) {
    if (screenOn == 0) {
      enableScreen(1);
    }
    if (frames % 2 == 0) {
      screenInfo();
    }
  }
  else {
    if (screenOn == 1) {
      enableScreen(0);
    }
  }
}

void screenInfo() {

  /* First line */
  lcd.setCursor(0,0);
  if (screenID == 1) {
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
    String local = String(localSteps);
    lcd.print(local);
    for (int i = 1; i < 16 - local.length(); i++) {
      lcd.print(F(" "));
    }
  }

  /* Second Line */
  lcd.setCursor(0,1);
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
