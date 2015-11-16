/******************************************************************/
/*
 *    @file: Fitbrick
 *    @authors: Matias Lopez, Raiyan Sobhan
 *
 */
/******************************************************************/

// Loading Libraries
#include <Wire.h>     // I2C
#include <SPI.h>      // SD Card
#include <SD.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_RGBLCDShield.h>

// Initialize Peripherals
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_MMA8451      mma = Adafruit_MMA8451();

// LCD Variables
#define LCD_ON 0x7
#define LCD_OFF 0x0
#define TIMEOUT 5000
#define REFRESH 30
long backlight = millis();
bool rfshlight = 0;
bool dbnclight = 0;

// SD Variables
File root;

// User Interface Variables
int menu = 0;

// The Real Deal
int totalSteps;
int steps = 0;
int activity = 0;

void setup() {
  // Debugging
  Serial.begin(9600);

  // Set LCD
  lcd.begin(16, 2);
  lcd.setBacklight(LCD_ON);
  lcd.setCursor(0, 0);
  lcd.print("Initializing");

  // Set MMA
  if (! mma.begin()) {
    Serial.println("Couldnt start MMA");
    lcd.setCursor(0, 0);
    lcd.print("Error: MMA Malfunction");
    while (1);
  }


  // Set SD Card
  if (!SD.begin()) {
    Serial.println("Couldnt start SD");
    lcd.setCursor(0, 0);
    lcd.print("Error: No SD");
    while (1);
  }

  // Reload
  readEvent();
}

void loop() {
  // User Interface
  uint8_t buttons = lcd.readButtons();
  if (buttons && (millis()-backlight) > 100) {
    if (buttons & BUTTON_LEFT) {

    }
    else if (buttons & BUTTON_RIGHT) {
      
    }
    else if (buttons & BUTTON_SELECT) {
      
    }
    backlight = millis();
    dbnclight = 1;
  }

  // LCD Refresh
  if ((millis() - backlight) < TIMEOUT) {
    if ((millis() - backlight) % round(TIMEOUT/REFRESH) < REFRESH && !rfshlight) {
      rfshlight = 1;
      
      // TODO
      
    }
    else if ((millis() - backlight) % round(TIMEOUT/REFRESH) > REFRESH && rfshlight) {
      rfshlight = 0;
    }
  }
  else if (dbnclight) {
    lcd.setBacklight(LCD_OFF);
    lcd.clear();
    dbnclight = 0;
  }
}

void readEvent() {
  root = SD.open("fbrk_stp.csv");
  String received = "";
  int state = 0;
  char ch;
  while (root.available()) {
    ch = root.read();
    if (ch == '\n') {
      state = 1;
    }
    else if (ch == ',') {
      state = 0;
    }
    else if (state = 1) {
      received = "" + ch;
      state = 2;
    }
    else if (state != 0) {
      received += ch;
    }
  }
  totalSteps = received.toInt();
}

void writeEvent(int kind) {
  if (kind == 0) {
    root = SD.open("fbrk_act.csv");
    root.print(activity);
    root.print(",");
    root.println(millis());
  }
  else if (kind == 1) {
    root = SD.open("fbrk_stp.csv");
    totalSteps += steps;
    root.print(totalSteps);
    root.print(",");
    root.print(steps);
    root.print(",");
    root.println(millis());
    steps = 0;
  }
  root.close();
}

