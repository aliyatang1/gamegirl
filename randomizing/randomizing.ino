/*
This is the foundation code for our future code. This randomizes the commands between "left" and "right". 
We need to add: 
1. progress bar
2. ESP-Now communication
*/

#include <TFT_eSPI.h>  
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); 

#define ARRAY_SIZE 2
const String commandVerbs[ARRAY_SIZE] = {"Left", "Right"};

// Define button pins
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

String currentCommand = ""; 

void setup() {
  Serial.begin(115200); 

  tft.init();
  tft.setRotation(0); 
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_GREEN, TFT_BLACK); 
  tft.setTextSize(2); 

  // Initialize buttons
  pinMode(BUTTON_LEFT, INPUT_PULLUP);  
  pinMode(BUTTON_RIGHT, INPUT_PULLUP); 

  // Display the first random command
  currentCommand = genCommand();
  displayCommand(currentCommand);
}

void loop() {
  if (digitalRead(BUTTON_LEFT) == LOW && currentCommand == "Left") {
    Serial.println("Correct! Left pressed.");
    refreshCommand();
  } else if (digitalRead(BUTTON_RIGHT) == LOW && currentCommand == "Right") {
    Serial.println("Correct! Right pressed.");
    refreshCommand();
  }
}

String genCommand() {
  return commandVerbs[random(ARRAY_SIZE)];
}

void displayCommand(String command) {
  tft.fillScreen(TFT_BLACK); 
  tft.drawString(command, 50, 100, 2); 
  Serial.println("Command: " + command); 
}

void refreshCommand() {
  currentCommand = genCommand();
  displayCommand(currentCommand);
}
