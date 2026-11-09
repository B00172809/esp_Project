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

int menuOption = 0;                        // Which (of 3) options is highlighted on the menu (0 = pressure hPa, 1 = pressure mbar, 2 = temperature C)
int currentPage = 0;                       // Which page is displayed (0 = menu, 1 = pressure hPa, 2 = pressure mbar, 3 = temperature C)
bool lastButton = HIGH;                    // Detects joystick click (display highlighted option)

//
unsigned long lastMove = 0;
unsigned long lastUpdate = 0;

// Display the main menu
void showMenu() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  tft.println("Main Menu");
  tft.println();

  const char* options[] = {
    "Pressure hPa",
    "Pressure mbar",
    "Temperature C"
  };

  for (int i = 0; i < 3; i++) {
    if (i == menuOption) {
      tft.print("> ");
    }
    else {
      tft.print("  ");
    }

    tft.println(options[i]);
  }
}

// Display the selected sensor page
void showPage() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  if (currentPage == 1) {
    tft.println("Pressure (hPa)");
    tft.println();
    tft.println(bmp.readPressure() / 100.0);
  }
  else if (currentPage == 2) {
    tft.println("Pressure (mbar)");
    tft.println();
    tft.println(bmp.readPressure() / 100.0);
  }
  else if (currentPage == 3) {
    tft.println("Temperature (C)");
    tft.println();
    tft.println(bmp.readTemperature());
  }

  tft.println();
  tft.println("Press to return");
}

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
  if (currentPage == 0 && millis() - lastMove > 250) {

    if (yValue < 1400) {
      menuOption++;
      if (menuOption > 2) {
        menuOption = 0;
      }
      showMenu();
      lastMove = millis();
    }
    else if (yValue > 2700) {
      menuOption--;
      if (menuOption < 0) {
        menuOption = 2;
      }
      showMenu();
      lastMove = millis();
    }
  }

  // Select page or return to menu
  if (buttonState == LOW && lastButton == HIGH) {
    if (currentPage == 0) {
      currentPage = menuOption + 1;
      showPage();
    }
    else {
      currentPage = 0;
      showMenu();
    }
  }

  lastButton = buttonState;

  // Update sensor reading every second
  if (currentPage != 0 && millis() - lastUpdate >= 1000) {
    showPage();
    lastUpdate = millis();
  }
}