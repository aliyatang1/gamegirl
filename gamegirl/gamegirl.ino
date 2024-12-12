/**
What we need to finish:
1. need to work on the graphics
2. making sure text size is good
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

bool gameStarted = false; 

void setup() {
  Serial.begin(115200); 
  tft.init();
  tft.setRotation(1); 

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
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("ESP-NOW Broadcast Demo");

  // Print MAC address
  Serial.print("MAC Address: ");
  String mac = WiFi.macAddress();
  if (mac == "00:00:00:00:00:00") {
    Serial.println("Error: Unable to retrieve MAC Address. Retrying...");
    delay(1000);
    ESP.restart();  
  } else {
    Serial.println(mac);
  }

  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
    addBroadcastPeer();
  } else {
    Serial.println("ESP-NOW Init Failed. Restarting...");
    delay(3000);
    ESP.restart();  
  }
}

void addBroadcastPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false; 

  if (!esp_now_is_peer_exist(broadcastAddress)) {
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("Broadcast peer added");
    } else {
      Serial.println("Failed to add broadcast peer");
    }
  }
}

void gameStart() {
  gameStarted = true;
  progress = 0;
  tft.setTextSize(2); 
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
  if (!gameStarted) {
    static bool messageDisplayed = false;
    if (!messageDisplayed) {
      tft.setTextSize(1.75); 
      displayCommand("Press both buttons to start");
      messageDisplayed = true;
    }
    if (digitalRead(BUTTON_LEFT) == LOW && digitalRead(BUTTON_RIGHT) == LOW) {
      messageDisplayed = false;  
      gameStart();
    }
  }

  // If the game has started, process the commands
  if (gameStarted) {
    if ((digitalRead(BUTTON_LEFT) == LOW && currentCommand == "Left") && digitalRead(BUTTON_RIGHT) != LOW) {
      Serial.println("Left pressed.");
      delay(100);
      progress += 1; 
      sendProgress();
      if (progress >= 30) {
        displaySuccessMessage();
      } else {
        displayProgressBar();
        refreshCommand();
      }
    } else if ((digitalRead(BUTTON_RIGHT) == LOW && currentCommand == "Right") && digitalRead(BUTTON_LEFT) != LOW) {
      Serial.println("Right pressed.");
      delay(100);
      progress += 1; 
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
  return commandVerbs[random(2)];
}

void displayCommand(String command) {
  int textWidth = tft.textWidth(command, 2);  
  int textHeight = 16; 

  int x = (screenWidth - textWidth) / 2;
  int y = 3 * (screenHeight - textHeight) / 4;
  tft.fillRect(0, y, screenWidth, textHeight + 20, TFT_WHITE);
  tft.drawString(command, x, y, 2); 
  Serial.println("Command: " + command); 
}

void displayProgressBar() {
  int barWidth = screenWidth - 100; 
  int barHeight = 20; 
  int maxProgress = 30; 
  int progressWidth = map(progress, 0, maxProgress, 0, barWidth);

  int x = (screenWidth - barWidth) / 2;
  int y = 30;

  tft.fillRect(x, y, barWidth, barHeight, TFT_PINK);

  tft.fillRect(x, y, progressWidth, barHeight, TFT_RED);
}

// Callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// Callback when data is received
void receiveCallback(const esp_now_recv_info_t *info, const uint8_t *data, int dataLen) {
  Serial.printf("Received data: ");
  for (int i = 0; i < dataLen; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();

  if (dataLen == 1) { 
    int otherPlayerProgress = data[0];
    Serial.printf("Other Player Progress: %d\n", otherPlayerProgress);

    if (otherPlayerProgress >= 28) {
      displayFailedMessage();  
    }
  } else {
    Serial.println("Unexpected data length");
  }
}

void refreshCommand() {
  currentCommand = genCommand();
  displayCommand(currentCommand);
}

void sendProgress() {
  uint8_t progressData = progress;  
  esp_now_send(broadcastAddress, &progressData, sizeof(progressData));
}

void displaySuccessMessage() {
  textSetUp();
  tft.fillScreen(TFT_GREEN);  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(50, screenHeight / 2);
  tft.println("Success");
  delay(3000);  
  resetGame();
}

void displayFailedMessage() {
  textSetUp();
  tft.fillScreen(TFT_RED);  
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(50, screenHeight / 2);
  tft.println("Failed");
  delay(3000);  
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
  game();  
}
