#include <TFT_eSPI.h>  
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); 

const String commandVerbs[2] = {"Left", "Right"};

// Define button pins
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

String currentCommand = ""; 
int progress = 0;

int screenWidth;
int screenHeight;

void setup() {
  Serial.begin(115200); 
  tft.init();
  tft.setRotation(1); // Horizontal 

  // Initialize screen dimensions after tft.init()
  screenWidth = tft.width();
  screenHeight = tft.height();

  textSetUp();
  buttonSetUp();

  // Display the first random command
  currentCommand = genCommand();
  displayCommand(currentCommand);
}

void textSetUp() {
  tft.fillScreen(TFT_WHITE); 
  tft.setTextColor(TFT_RED); 
  tft.setTextSize(2); 
}

void buttonSetUp() {
  pinMode(BUTTON_LEFT, INPUT_PULLUP);  
  pinMode(BUTTON_RIGHT, INPUT_PULLUP); 
}

void loop() {
  if (digitalRead(BUTTON_LEFT) == LOW && currentCommand == "Left") {
    Serial.println("Correct! Left pressed.");
    delay(100);
    progress += 1; // Increment progress
    displayProgressBar(); // Update progress bar
    refreshCommand();
  } else if (digitalRead(BUTTON_RIGHT) == LOW && currentCommand == "Right") {
    Serial.println("Correct! Right pressed.");
    delay(100);
    progress += 1; // Increment progress
    displayProgressBar(); // Update progress bar
    refreshCommand();
  }
}

String genCommand() {
  // Generates random command 
  int randInt = random(2);
  Serial.println("Random number: " + String(randInt));
  return commandVerbs[randInt];
}

void displayCommand(String command) {
  int textWidth = tft.textWidth(command, 2); // Use font size 2
  int textHeight = 16; // Approximate height of font size 2

  // Calculate centered coordinates
  int x = (screenWidth - textWidth) / 2;
  int y = 3 * (screenHeight - textHeight) / 4;

  // Clear the area with a white rectangle
  tft.fillRect(x-10, y, textWidth + 30, textHeight + 100, TFT_WHITE);

  // Draw the string at the calculated position
  tft.drawString(command, x, y, 2); 
  Serial.println("Command: " + command); 
}

void displayProgressBar() {
  int barWidth = screenWidth - 100; // Width of the progress bar
  int barHeight = 20; // Height of the progress bar
  int maxProgress = 30; // Maximum progress value (adjust as needed)

  // Calculate the width of the progress bar based on the current progress
  int progressWidth = map(progress, 0, maxProgress, 0, barWidth);

  // Calculate the position of the progress bar
  int x = (screenWidth - barWidth) / 2;
  int y = 30;

  // Clear the previous progress bar
  tft.fillRect(x, y, barWidth, barHeight, TFT_PINK);

  // Draw the new progress bar
  tft.fillRect(x, y, progressWidth, barHeight, TFT_RED);
  Serial.println("Progress: " + String(progress));
}

void refreshCommand() {
  currentCommand = genCommand();
  displayCommand(currentCommand);
}
