/**
Tasks to be implemented:

3. Graphics

1.5 making it multiplayer through the ESP-Now - Good (?) need to test!
2. Home Screen - Good!
4. Add condition where if two buttons are pressed simultaneously, nothing happens - Good!
**/

#include <TFT_eSPI.h>  
#include <SPI.h>
#include <esp_now.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI(); 

const String commandVerbs[2] = {"Left", "Right"};

// Define button pins
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

String currentCommand = ""; 
int progress = 0;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
  espnowSetup();
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

void espnowSetup() {
  // Set ESP32 in STA mode to begin with
  delay(500);
  WiFi.mode(WIFI_STA);
  Serial.println("ESP-NOW Broadcast Demo");

  // Print MAC address
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Disconnect from WiFi
  WiFi.disconnect();

  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW Init Success");
    //Handles incoming messages via ESP-NOW
    esp_now_register_recv_cb(receiveCallback);

    //Sends messages via ESP-NOW
    esp_now_register_send_cb(sentCallback);
    broadcastMessage();
  } else {
    Serial.println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }
}

void gameStart() {
  gameStarted = true;
  progress = 0;
  tft.setTextSize(2); 
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
  tft.setTextSize(1.75); 
  //prevent text flickering
  if (!gameStarted) {
    static bool messageDisplayed = false;
    if (!messageDisplayed) {
      tft.setTextSize(1.75); 
      displayCommand("Press both buttons to start");
      messageDisplayed = true;
    }
    if (digitalRead(BUTTON_LEFT) == LOW && digitalRead(BUTTON_RIGHT) == LOW) {
        messageDisplayed = false;  // Reset for future use
        gameStart();
    }
  }

  // If the game has started, process the commands
  if (gameStarted) {
    // the additional BUTTON_RIGHT != LOW prevents the individual from cheating on the game by pressing two buttons simultaneously
    if ((digitalRead(BUTTON_LEFT) == LOW && currentCommand == "Left")&& digitalRead(BUTTON_RIGHT) != LOW) {
      Serial.println("Left pressed.");
      delay(100);
      progress += 1; // Increment progress
      //sending the other player that the other player won
      sendProgress();
      if (progress >= 30) {
        displaySuccessMessage();
      } else {
        displayProgressBar();
        refreshCommand();
      }
    // the additional BUTTON_LEFT != LOW prevents the individual from cheating on the game by pressing two buttons simultaneously
    } else if ((digitalRead(BUTTON_RIGHT) == LOW && currentCommand == "Right") && digitalRead(BUTTON_LEFT) != LOW) {
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
  tft.setTextSize(2); 
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
  //get the progress -> need to send it to the other ESP-32 to see if they already got to 100% 
  String progressStr = String(map(progress, 0, maxProgress, 0, 100)) + "%";

  // If you want to draw percentage: 
 // 
 // int textWidth = tft.textWidth(progressStr);
 // int textHeight = 16;
 // int textX = (screenWidth - textWidth) / 2;
 // int textY = y + barHeight + 5;
  
//  tft.setTextColor(TFT_BLACK);
//  tft.setTextSize(2);
//  tft.drawString(progressStr, textX, textY);
//  Serial.println("Progress: " + String(progress));
}
// Callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// Callback when data is received
void receiveCallback(const esp_now_recv_info_t *info, const uint8_t *data, int dataLen) {
  // Convert received data to a string
  String receivedMessage = String((char *)data);
  if (receivedMessage == "DISCOVERY") {
    Serial.println("Discovery message received!");

    // Respond with this device's MAC address
    uint8_t myMacAddress[6];
    WiFi.macAddress(myMacAddress);
    esp_now_send(info->src_addr, myMacAddress, 6);
  } else {
    // Handle other game-related messages
    int otherPlayerProgress = data[0];
    Serial.printf("Other Player Progress: %d\n", otherPlayerProgress);

    if (otherPlayerProgress >= 30) {
      displayFailedMessage();  // The other player won
    }
  }
}

// I asked ChatGPT to help with this specific part because I'm not sure what to do about the peer callback -> modified it into the dicovery broadcast message
void broadcastMessage() {
  const char *message = "DISCOVERY";
  esp_now_send(broadcastAddress, (uint8_t *)message, strlen(message));
}
void refreshCommand() {
  currentCommand = genCommand();
  displayCommand(currentCommand);
}
void sendProgress() {
  uint8_t progressData = progress;  // Convert progress to a single byte
  esp_now_send(broadcastAddress, &progressData, sizeof(progressData));
}

void displaySuccessMessage() {
  tft.fillScreen(TFT_GREEN);  // Clear the screen
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, screenHeight / 2);
  tft.println("Success");
  delay(3000);  // Wait for the message to show for 3 seconds
  resetGame();
}
void displayFailedMessage(){
  tft.fillScreen(TFT_RED);  // Clear the screen
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, screenHeight / 2);
  tft.println("Failed");
  delay(3000);  // Wait for the message to show for 3 seconds
  resetGame();

}
void resetGame() {
  gameStarted = false;
  progress = 0;
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(2); 
  tft.setTextColor(TFT_RED);
  tft.println("Press both buttons to start");
}


void loop() {
  game();  // Call the game loop
}
