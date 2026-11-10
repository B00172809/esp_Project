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
const int joyClick = 7;


/* Block 3: Variables */
//Create objects for display (tft) and sensor (bmp) from Library classes
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP280 bmp;

int highlightOption = 0;                        // Which (of 3) options is highlighted on the menu (0 = pressure hPa, 1 = pressure mbar, 2 = temperature C)
int openPage = 0;                               // Which page is displayed (0 = menu, 1 = pressure hPa, 2 = pressure mbar, 3 = temperature C)
bool lastButton = HIGH;                         // Detects joystick click (display highlighted option)

unsigned long lastMove = 0;                     // Stores time of last joystick movement (to prevent fast scrolling)
unsigned long lastUpdate = 0;                   // Stores time of last sensor display update (refresh per 1 sec)


/* Block 4: Main Menu */
void showMenu() {
  tft.fillScreen(ST77XX_BLACK);                 //clears screen
  tft.setCursor(0, 20);                         // positions text top left corner
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


/* Block 6: Initialization */
void setup() {
  Serial.begin(115200);

  // Initialise joystick
  pinMode(joyClick, INPUT_PULLUP);              // Joystick Clicked = LOW, Not Clicked = HIGH
  analogReadResolution(12);                     // ADC is set to 12 bits as joyY and joyX are analog but joyClick is digital

  // Initialise screen
  SPI.begin();                                
  tft.init(240, 320);                          // Display [Width, Height] in pixels
  tft.fillScreen(ST77XX_BLACK);                //Clears screen on startup to prevent undesierables

  // Initialise BMP280
  Wire.begin(I2C_SDA, I2C_SCL);               //Initialises the pins the BMP280 sensor physically connects to on the i2c


/* Block 7:BMP280 Detection test + Display Menu*/
  if (!bmp.begin(0x76)) {                    //BMP280 sensor address is 0x76, uses ! so code continues on if it can begin but if it cant:
    tft.println("BMP280 not found");        //prints on Screen if sensor is not detected
    Serial.println("BMP280 not found");     //Does the same on Serial Montitor in case the screen is the issue
    
    //Endless delay so code wont continue without the BMP detection
    while (1) {
      delay(1000);
    }
  }

  //with the risk eliminated of bad sensor detection, menu shows on screen
  showMenu();                              
}


/* Block 8: Main Loop */
void loop() {
  int yAxis = analogRead(joyY);                                 //Stores joystick y-axis value (0-4095) inside int
  bool buttonState = digitalRead(joyClick);                     //Reads joystick state: Low = Clicked, High = Not Clicked

  // If main menu is dispalyed, & 250ms+ since last joystick movement: read joystick y-axis value to navigate menu
  if (openPage == 0 && (millis() - lastMove) > 250) {            //millis counts milliseconds

    if (yAxis < 1400) {                                          //if joystick moves downward:
      highlightOption++;                                         //highlight the option below it
      if (highlightOption > 2) {                                 //if joystick descends on bottom option, send back to top
        highlightOption = 0;
      }
      showMenu();
      lastMove = millis();
    }
    else if (yAxis > 2700) {                                    //if joystick moves upwards:
      highlightOption--;                                        //highlight the option above it
      if (highlightOption < 0) {                                //if joystick ascends on top option, send to bottom
        highlightOption = 2;
      }
      showMenu();                                               //update screen  to isplay the > beside the highlighted menu option
      lastMove = millis();                                      //store the current ms time as its now the new last joystick movement
    }
  }


/* Block 9:*/
  // Select page or return to menu
  if (buttonState == LOW && lastButton == HIGH) {               //if joystick is clicked (LOW) and it was not clicked before (HIGH)
    if (openPage == 0) {
      openPage = highlightOption + 1;                           //if clicking on main menu, open the highlighted options | +1 needed as main menu cant be highlighted
      showPage();                                               //update screen to the new openPage
    }
    else {
      openPage = 0;                                             //otherwise revert to main menu - stick click when already on one of the three pages
      showMenu();
    }
  }


/* Block 10: Button Memory*/
  lastButton = buttonState;                                     //store the current button state as its now the new last button state

  // Update sensor reading every second
  if (openPage != 0 && millis() - lastUpdate >= 1000) {         //if not on main menu and 1 second has passed since last update
    showPage();                                                 //refresh screen with new info
    lastUpdate = millis();                                      //store the current ms time as its now the new last update time
  }
}