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

bool gameStarted = false;  // To track if the game has started

void setup() {
  Serial.begin(115200); 
  tft.init();
  tft.setRotation(1); // landscape 

  // initialize dimentions
  screenWidth = tft.width();
  screenHeight = tft.height();

  textSetUp();
  buttonSetUp();
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

void gameStart() {
  tft.fillScreen(TFT_WHITE);
  gameStarted = true;
  progress = 0;

  // Displaying a message 
  displayCommand("Get ready...");
  delay(1000);
  displayCommand("3");
  delay(500);
  displayCommand("2");
  delay(500);
  displayCommand("1");
  delay(500);
  displayCommand("Race!");
  currentCommand = genCommand();
  displayCommand(currentCommand);
}

void game() {
  // Check if both buttons are pressed to start the game
  if (!gameStarted && digitalRead(BUTTON_LEFT) == LOW && digitalRead(BUTTON_RIGHT) == LOW) {
    gameStart();
  }

  // If the game has started, process the commands
  if (gameStarted) {
    if (digitalRead(BUTTON_LEFT) == LOW && currentCommand == "Left") {
      Serial.println("Left pressed.");
      delay(100);
      progress += 1; // Increment progress
      if (progress >= 30) {
        displaySuccessMessage();
      } else {
        displayProgressBar();
        refreshCommand();
      }
    } else if (digitalRead(BUTTON_RIGHT) == LOW && currentCommand == "Right") {
      Serial.println("Right pressed.");
      delay(100);
      progress += 1; // Increment progress
      if (progress >= 30) {
        displaySuccessMessage();
      } else {
        displayProgressBar();
        refreshCommand();
      }
    }
  }
}

String genCommand() {
  // Generates random command 
  int randInt = random(2);
  Serial.println("Random number: " + String(randInt));
  return commandVerbs[randInt];
}

void displayCommand(String command) {
  int textWidth = tft.textWidth(command, 2);  
  int textHeight = 16; // Approximate height of font size 2

  // Calculate centered coordinates
  int x = (screenWidth - textWidth) / 2;
  int y = 3 * (screenHeight - textHeight) / 4;

  // Clear the area with a white rectangle
  tft.fillRect(0, y,5000, textHeight + 100, TFT_WHITE);

  // Draw the string at the calculated position
  tft.drawString(command, x, y, 2); 
  Serial.println("Command: " + command); 
}

void displayProgressBar() {
  int barWidth = screenWidth - 100; // Width of the progress bar
  int barHeight = 20; 
  int maxProgress = 50; 

  // Calculate the width of the progress bar based on the current progress
  int progressWidth = map(progress, 0, maxProgress, 0, barWidth);

  // Position
  int x = (screenWidth - barWidth) / 2;
  int y = 30;

  // Clear the previous progress bar
  tft.fillRect(x, y, barWidth, barHeight, TFT_PINK);
  // Draw the new progress bar
  tft.fillRect(x, y, progressWidth, barHeight, TFT_RED);

  // If you want to draw percentage: 
 // String progressStr = String(map(progress, 0, maxProgress, 0, 100)) + "%";
 // int textWidth = tft.textWidth(progressStr);
 // int textHeight = 16;
 // int textX = (screenWidth - textWidth) / 2;
 // int textY = y + barHeight + 5;
  
//  tft.setTextColor(TFT_BLACK);
//  tft.setTextSize(2);
//  tft.drawString(progressStr, textX, textY);
//  Serial.println("Progress: " + String(progress));
}

void refreshCommand() {
  currentCommand = genCommand();
  displayCommand(currentCommand);
}

void displaySuccessMessage() {
  tft.fillScreen(TFT_GREEN);  // Clear the screen
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, screenHeight / 2);
  tft.println("Success");
  delay(5000);  // Wait for the message to show for 5 seconds
  resetGame();
}

void resetGame() {
  gameStarted = false;
  progress = 0;
  displayCommand("Press both buttons to start");
}



void loop() {
  game();  // Call the game loop
}
