# Typing Game on Gamegirl

Welcome to our multi-player game designed specifically for embedded devices! The objective is straightforward yet competitive: outpace your opponent and win the race. Inspired by [Nitro-Type](https://www.nitrotype.com), our game focuses on generating commands like **"Left"** or **"Right"**. Players compete to hit the correct buttons for the commands as quickly and accurately as possible. The player who successfully fills up their progress bar first wins the game! 

### Authors: 
Aliya Tang and Mila Lin Tabach 
### Hardware Components: 
- 4 buttons (2 buttons for each Gamegirl enclosure)
- USB-C cables
- Wires

<div align="center">
  <img src="components_picture.jpg" alt="Components Picture" width="300"/>
</div>

## Demo

## Overview
Our code sets up a multiplayer game using ESP-NOW with a TFT display on ESP32. The game revolves around pressing buttons (left and right) to match randomly generated commands like "Left" or "Right." Players compete to reach a progress goal (30) first. If either player reaches a specific threshold (28 or more), they win and is shown a “Success” message while the other is shown a "Failed" message. The codebase includes different components including ESP-NOW communication, progress bar, and the gaming logic. 

### Libraries to Download:
- TFT_eSPI
- SPI
- esp_now
- WiFi





