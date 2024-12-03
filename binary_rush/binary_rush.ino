/*
  Binary Rush on a Gamegirl (Multiplayer Mode)
  Written by: Mila and Aliya!
*/

// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>  // Graphics and font library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

String cmd1 = "";
String cmd2 = "";
String cmdRecvd = "";
const String waitingCmd = "Wait for cmds";
bool redrawCmdRecvd = false;

// Player Variables
int player1Progress = 0;
int player2Progress = 0; 
bool opponentWon = false;

bool isPlayer1 = true;

// For drawing progress bars
bool redrawProgress = true;

// The array is for two separate commands "left" or "right"
#define ARRAY_SIZE 2
const String commandVerbs[ARRAY_SIZE] = { "Left", "Right" };

int lineHeight = 30;

// Define LED and pushbutton pins
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

// ESP-NOW Callbacks
void onDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Data sent successfully" : "Data send failed");
}

void onDataReceived(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  String message = String((char*)data);
  Serial.println("Message received: " + message);

  // Parse the received message
  if (message.startsWith("P:")) {
    player2Progress = message.substring(2).toInt();
  } else if (message == "WIN") {
    opponentWon = true;
  }
}

// Setup ESP-NOW
void setupESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    delay(2000);  // Allow time for the message to print
    ESP.restart();
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceived);
}

void sendData(const String &message) {
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
}

void buttonSetup() {
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
}

void textSetup() {
  tft.init();
  tft.setRotation(0);

  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  drawControls();
  
  tft.fillRect(0, 0, 135, 65, TFT_BLACK); 

  cmdRecvd = waitingCmd;
  redrawCmdRecvd = true;
}

String genCommand() {
  String verb = commandVerbs[random(ARRAY_SIZE)];
  return verb;
}

void drawControls() {
  cmd1 = genCommand();
  cmd2 = genCommand();

  // Draw the button commands directly
  tft.fillRect(0, 115, 135, 50, TFT_BLACK);  // Clear previous commands area
  tft.setTextColor(TFT_GREEN);
  tft.drawString("B1: " + cmd1, 0, 115, 2);
  tft.drawString("B2: " + cmd2, 0, 175, 2);
}

void setup() {
  Serial.begin(115200);

  textSetup();
  buttonSetup();
  setupESPNow();
}

void loop() {
  // Check for button presses and compare with commands
  if (digitalRead(BUTTON_LEFT) == LOW && cmdRecvd == cmd1) {
    player1Progress += 10;
    sendData("P:" + String(player1Progress)); // Send progress to opponent
    cmdRecvd = waitingCmd;
    redrawProgress = true;
  } else if (digitalRead(BUTTON_RIGHT) == LOW && cmdRecvd == cmd2) {
    player1Progress += 10;
    sendData("P:" + String(player1Progress)); // Send progress to opponent
    cmdRecvd = waitingCmd;
    redrawProgress = true;
  }

  // Draw progress bars for both players
  if (redrawProgress) {
    // Clear the progress bar area
    tft.fillRect(15, lineHeight * 2 + 5, 100, 20, TFT_BLACK);
    tft.fillRect(15, lineHeight * 2 + 15, 100, 20, TFT_BLACK);

    // Player 1 progress
    tft.fillRect(15, lineHeight * 2 + 5, 100, 6, TFT_GREEN);
    tft.fillRect(16, lineHeight * 2 + 5 + 1, player1Progress, 4, TFT_BLUE);

    // Player 2 progress
    tft.fillRect(15, lineHeight * 2 + 15, 100, 6, TFT_RED);
    tft.fillRect(16, lineHeight * 2 + 15 + 1, player2Progress, 4, TFT_YELLOW);

    redrawProgress = false;
  }

  // Check for win condition
  if (player1Progress >= 100 && !opponentWon) {
    tft.fillScreen(TFT_BLUE);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("You", 45, 20, 2);
    tft.drawString("Won!", 20, 80, 2);

    sendData("WIN"); // Notify opponent of win
    delay(6000);
    ESP.restart();
  } else if (opponentWon) {
    tft.fillScreen(TFT_RED);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("You", 45, 20, 2);
    tft.drawString("Lost!", 20, 80, 2);

    delay(6000);
    ESP.restart();
  }
}
