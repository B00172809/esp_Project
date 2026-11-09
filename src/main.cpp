/* Block 1: Libraries */
#include <Arduino.h>                        //basic arduino functions
#include <Wire.h>                           //Communicates to BMP Sensor using i2c 
#include <SPI.h>                            //For screen communication using SPI
#include <Adafruit_GFX.h>                   //Functions to draw graphics
#include <Adafruit_ST7789.h>                //Controls our display
#include <Adafruit_BMP280.h>                //Reads values from sensor

/* Block 2: Pin Defines */
//Pins on TFT as per GPIO values on physical board
#define TFT_CS 4
#define TFT_RST 6
#define TFT_DC 5

//Pins on BMP280 as per esp32 GPIO values on physical board
#define I2C_SDA 8
#define I2C_SCL 9

//Pins on Joystick as per esp32 GPIO values on physical board
const int joyX = 1;
const int joyY = 2;
const int joyButton = 7;


/* Block 3: Variables */
//Create objects for display (tft) and sensor (bmp) from Library classes
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP280 bmp;

int highlightOption = 0;                        // Which (of 3) options is highlighted on the menu (0 = pressure hPa, 1 = pressure mbar, 2 = temperature C)
int openPage = 0;                       // Which page is displayed (0 = menu, 1 = pressure hPa, 2 = pressure mbar, 3 = temperature C)
bool lastButton = HIGH;                    // Detects joystick click (display highlighted option)

unsigned long lastMove = 0;                // Stores time of last joystick movement (to prevent fast scrolling)
unsigned long lastUpdate = 0;              // Stores time of last sensor display update (refresh per 1 sec)


/* Block 4: Main Menu */
void showMenu() {
  tft.fillScreen(ST77XX_BLACK);           //clears screen
  tft.setCursor(0, 20);                   // positions text top left corner
  tft.setTextSize(2);                     
  tft.setTextColor(ST77XX_WHITE);

  tft.println("Main Menu\n");

  //The options on screen to be selected - stored in array
  const char* options[] = {
    "Pressure hPa",
    "Pressure mbar",
    "Temperature C"
  };

  //prints menu options + highlights the selected one
  for (int i = 0; i < 3; i++) {
    if (i == highlightOption) {
      tft.print("> ");
    }
    else {
      tft.print("  ");
    }

    tft.println(options[i]);
  }
}

/* Block 5: Selected Pages*/
//default configurations for all three pages
void showPage() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  if (openPage == 1) {
    tft.println("Pressure (hPa)\n");
    tft.println(bmp.readPressure() / 100.0);
  }
  else if (openPage == 2) {
    tft.println("Pressure (mbar)\n");
    tft.println(bmp.readPressure() / 100.0);
  }
  else if (openPage == 3) {
    tft.println("Temperature (C)\n");
    tft.println(bmp.readTemperature());
  }

  tft.println("\nPress to return");
}

/* Block 6:*/
void setup() {
  Serial.begin(115200);

  // Initialise joystick
  pinMode(joyButton, INPUT_PULLUP);
  analogReadResolution(12);

  // Initialise screen
  SPI.begin();
  tft.init(240, 320);
  tft.fillScreen(ST77XX_BLACK);

  // Initialise BMP280
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found");
    tft.println("BMP280 not found");
    while (1) {
      delay(1000);
    }
  }

  showMenu();
}

void loop() {
  int yValue = analogRead(joyY);
  bool buttonState = digitalRead(joyButton);

  // Navigate menu using joystick
  if (openPage == 0 && millis() - lastMove > 250) {

    if (yValue < 1400) {
      highlightOption++;
      if (highlightOption > 2) {
        highlightOption = 0;
      }
      showMenu();
      lastMove = millis();
    }
    else if (yValue > 2700) {
      highlightOption--;
      if (highlightOption < 0) {
        highlightOption = 2;
      }
      showMenu();
      lastMove = millis();
    }
  }

  // Select page or return to menu
  if (buttonState == LOW && lastButton == HIGH) {
    if (openPage == 0) {
      openPage = highlightOption + 1;
      showPage();
    }
    else {
      openPage = 0;
      showMenu();
    }
  }

  lastButton = buttonState;

  // Update sensor reading every second
  if (openPage != 0 && millis() - lastUpdate >= 1000) {
    showPage();
    lastUpdate = millis();
  }
}