// Libraries
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <EEPROM.h>
#include "gamemenu.h"
#include "inputhandler.h"
#include "spaceinvador.h"
#include "flappybird.h"
#include "snakegame.h"
#include "breakout.h"
// Pin definitions for TFT display
#define TFT_CS D0 // Chip Select
#define TFT_RST D1 // Reset
#define TFT_DC D2 // Data/Command
#define TFT_MOSI D3
#define TFT_SCLK D4
#define TFT_LED D5

// Joystick pins
#define X_PIN 5 // Analog pin A0
#define Y_PIN 4 // Analog pin A1
#define Button_PIN D10
#define Vibrationmotor_PIN D9

// Initialize TFT display
// Increase SPI clock speed (check your display's specs for maximum supported speed!)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Game constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define PLAYER_WIDTH 11
#define PLAYER_HEIGHT 8
#define ALIEN_ROWS 3
#define ALIEN_COLS 6
#define ALIEN_WIDTH 8
#define ALIEN_HEIGHT 8
#define ALIEN_SPACING_X 12
#define ALIEN_SPACING_Y 12
#define BULLET_WIDTH 2
#define BULLET_HEIGHT 5
#define MAX_BULLETS 3
#define MAX_ALIEN_BULLETS 2
#define PLAYER_SPEED 2
#define BULLET_SPEED 3
#define SHIELD_COUNT 3
#define SHIELD_WIDTH 16
#define SHIELD_HEIGHT 8

// Colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xF800

// Game state
String gameState = "menu";


// Game instances
InputHandler inputHandler(Button_PIN, X_PIN, Y_PIN);
GameMenu gameMenu(tft, inputHandler);
SpaceInvador spaceInvador(tft, Button_PIN, Vibrationmotor_PIN, X_PIN, Y_PIN);
FlappyBird flappyBird(tft, Button_PIN, Vibrationmotor_PIN, X_PIN, Y_PIN);
SnakeGame snakeGame(&tft, &inputHandler);
Breakout breakoutGame(tft, Button_PIN, Vibrationmotor_PIN, X_PIN, Y_PIN);

void setup() {
  Serial.begin(9600);
  
  // Initialize display
  // You can try to set the SPI frequency here (in Hz)
  // Example: tft.begin(INITR_144GREENTAB, SPI_FREQUENCY);
  // Replace SPI_FREQUENCY with a value like 40000000 (40MHz).
  tft.initR(INITR_144GREENTAB);
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  tft.fillScreen(BLACK);
  
  // Initialize LED backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  
  // Initialize controls
  pinMode(Button_PIN, INPUT_PULLUP);
  pinMode(Vibrationmotor_PIN, OUTPUT);
  
  // Initialize game menu
  gameMenu.init();
}

void loop() {
  // Update input handler
  inputHandler.update();
  
  // Handle button press for menu navigation
  if (inputHandler.buttonPressed) {
    Serial.println("Button press detected in main loop");
    if (gameState == "menu") {
      Serial.println("Menu state detected");
      gameMenu.draw(); // Update menu to process button press
      
      if (gameMenu.shouldLaunchGame) {
        Serial.println("Launching game " + String(gameMenu.currentGameIndex));
        gameState = "game";
        tft.fillScreen(BLACK);
        
        // Launch the selected game
        if (gameMenu.currentGameIndex == 0) {
          // Space Invaders
          spaceInvador.init();
        } else if (gameMenu.currentGameIndex == 1) {
          // Flappy Bird
          flappyBird.init();
        } else if (gameMenu.currentGameIndex == 2) {
          // Snake Game
          snakeGame.init();
        } else if (gameMenu.currentGameIndex == 3) {
          // Breakout Game
          breakoutGame.init();
        }
        
        gameMenu.shouldLaunchGame = false;
        gameMenu.selectedItem = 0; // Reset menu selection
        inputHandler.reset(); // Clear all input states
        return; // Exit early to prevent multiple state changes
      }
    } else if (gameState == "game") {
      // Check if any game is over and return to menu
      if ((gameMenu.currentGameIndex == 0 && spaceInvador.getState() == SpaceInvador::GAME_OVER) ||
          (gameMenu.currentGameIndex == 1 && flappyBird.getState() == FlappyBird::GAME_OVER)) {
        gameState = "menu";
        tft.fillScreen(BLACK); // Clear screen before returning to menu
        inputHandler.buttonPressed = false; // Reset button state
        return; // Exit early to prevent multiple state changes
      }
    }
  }
  
  if (gameState == "menu") {
    // Handle menu navigation with joystick
    if (inputHandler.up) {
      gameMenu.selectedItem = max(0, gameMenu.selectedItem - 1);
    } else if (inputHandler.down) {
      gameMenu.selectedItem = min(MAX_MENU_ITEMS - 1, gameMenu.selectedItem + 1);
    }
    
    gameMenu.draw();
  } else if (gameState == "game") {
    // Update active game
    bool buttonPressed = digitalRead(Button_PIN) == HIGH;
    bool buttonReleased = digitalRead(Button_PIN) == LOW;
    
    // Update the appropriate game based on which one is active
    if (gameMenu.currentGameIndex == 0) {
      // Update Space Invaders game
      spaceInvador.update(buttonPressed, buttonReleased);
    } else if (gameMenu.currentGameIndex == 1) {
      // Update Flappy Bird game
      flappyBird.update(buttonPressed, buttonReleased);
    } else if (gameMenu.currentGameIndex == 2) {
      // Update Snake game
      snakeGame.update();
      if (snakeGame.isGameOver()) {
        if (buttonPressed) {
          gameState = "menu";
          gameMenu.init();
        }
      }
    } else if (gameMenu.currentGameIndex == 3) {
      // Update Breakout game
      breakoutGame.update(buttonPressed, buttonReleased);
      breakoutGame.render();
      if (breakoutGame.isGameOver()) {
        if (buttonPressed) {
          gameState = "menu";
          gameMenu.init();
        }
      }
    }
    
    // Game over handling is done in each game's respective update method
    // and checked in the button press handler above
  }
}
