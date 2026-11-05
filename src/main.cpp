#include <Arduino.h>

/*
  ESP32-S3 Joystick Test
  Reads joystick X, Y and button.
  Displays results in the Serial Monitor.
*/

// Joystick GPIO pins
const int joyX = 1;
const int joyY = 2;
const int joyButton = 7;

void setup() {

  // Start Serial Monitor
  Serial.begin(115200);

  // Configure joystick button
  pinMode(joyButton, INPUT_PULLUP);

  // Wait for Serial Monitor
  delay(1000);

  Serial.println();
  Serial.println("Joystick Test Started");
  Serial.println("---------------------");
}

void loop() {

  // Read joystick X and Y positions
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);

  // Read joystick button
  int buttonState = digitalRead(joyButton);

  // Display joystick values
  Serial.print("X: ");
  Serial.print(xValue);

  Serial.print(" | Y: ");
  Serial.print(yValue);

  Serial.print(" | Button: ");

  if (buttonState == LOW) {
    Serial.println("PRESSED");
  }
  else {
    Serial.println("RELEASED");
  }

  // Detect horizontal movement
  // Detect horizontal movement
if (xValue < 1400) {
  Serial.println("RIGHT");
}
else if (xValue > 2700) {
  Serial.println("LEFT");
}

  // Detect vertical movement
  if (yValue < 1400) {
    Serial.println("DOWN");
  }
  else if (yValue > 2700) {
    Serial.println("UP");
  }

  // Separate readings
  Serial.println("---------------------");

  // Update every 200 milliseconds
  delay(200);
}