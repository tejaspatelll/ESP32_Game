#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <EEPROM.h>
#include "snake.h"
#include "inputhandler.h"

class SnakeGame {
public:
  enum GameState {
    INTRO,
    PLAYING,
    GAME_OVER
  };

  SnakeGame(Adafruit_ST7735* display, InputHandler* input)
    : tft(display), input_handler(input), snake(input), currentState(INTRO), highScore(0) {
    // Calculate cell dimensions to fit screen while maintaining aspect ratio
    int maxCellWidth = (tft->width() - 4) / Snake::GRID_SIZE; // Leave 2px margin on each side
    int maxCellHeight = (tft->height() - 24) / Snake::GRID_SIZE; // Leave more space for score at bottom
    cellWidth = min(maxCellWidth, maxCellHeight);
    cellHeight = cellWidth; // Keep cells square
    
    // Calculate offset to center the game grid
    offsetX = (tft->width() - (Snake::GRID_SIZE * cellWidth)) / 2;
    offsetY = 4; // Increase top margin slightly
    
    memset(lastGrid, 0, sizeof(lastGrid));
  }

  void init() {
    EEPROM.begin(4); // Initialize EEPROM with 4 bytes for high score
    highScore = EEPROM.read(0) | (EEPROM.read(1) << 8); // Read high score from EEPROM
    currentState = INTRO;
    snake.reset();
    tft->fillScreen(ST77XX_BLACK);
    drawIntroScreen();
  }

  void update() {
    switch (currentState) {
      case INTRO:
        if (input_handler->buttonPressed) {
          currentState = PLAYING;
          snake.reset();
          tft->fillScreen(ST77XX_BLACK);
          drawBorder();
          input_handler->buttonPressed = false;
        }
        break;

      case PLAYING:
        snake.update();
        render();
        if (snake.isGameOver()) {
          currentState = GAME_OVER;
          int currentScore = snake.getScore();
          if (currentScore > highScore) {
            highScore = currentScore;
            // Save new high score to EEPROM
            EEPROM.write(0, highScore & 0xFF);
            EEPROM.write(1, (highScore >> 8) & 0xFF);
            EEPROM.commit();
          }
          drawGameOverScreen();
        }
        delay(150); // Game speed control
        break;

      case GAME_OVER:
        if (input_handler->buttonPressed) {
          currentState = INTRO;
          tft->fillScreen(ST77XX_BLACK);
          drawIntroScreen();
          input_handler->buttonPressed = false;
        }
        break;
    }
  }

  bool isGameOver() const {
    return currentState == GAME_OVER;
  }

private:
  GameState currentState;
  int highScore;

  void drawIntroScreen() {
    // Clear screen first
    tft->fillScreen(ST77XX_BLACK);
    
    // Draw retro-style border
    tft->drawRect(2, 2, tft->width()-4, tft->height()-4, ST77XX_YELLOW);
    
    // Draw title with shadow effect for retro look
    tft->setTextSize(2);
    tft->setTextColor(ST77XX_BLUE);
    tft->setCursor(17, 17);
    tft->print("SNAKE");
    tft->setTextColor(ST77XX_GREEN);
    tft->setCursor(15, 15);
    tft->print("SNAKE");
    
    // Draw decorative line
    for(int i = 10; i < tft->width()-10; i+=4) {
      tft->drawPixel(i, 40, ST77XX_YELLOW);
    }
    
    // Display high score with retro styling
    tft->setTextSize(1);
    tft->setTextColor(ST77XX_CYAN);
    tft->setCursor(25, 55);
    tft->print("HIGH SCORE");
    tft->setTextSize(2);
    tft->setCursor(35, 70);
    tft->print(highScore);
    
    // Blinking start message
    tft->setTextSize(1);
    if((millis() / 500) % 2) {
      tft->setTextColor(ST77XX_WHITE);
      tft->setCursor(15, 100);
      tft->print("PRESS TO START!");
    }
    
    // No need to force display update - it happens automatically
  }

  void drawGameOverScreen() {
    tft->fillScreen(ST77XX_BLACK);
    
    // Draw retro-style border
    tft->drawRect(2, 2, tft->width()-4, tft->height()-4, ST77XX_RED);
    
    // Draw "GAME OVER" with shadow effect
    tft->setTextSize(2);
    tft->setTextColor(ST77XX_BLUE);
    tft->setCursor(17, 27);
    tft->print("GAME OVER");
    tft->setTextColor(ST77XX_RED);
    tft->setCursor(15, 25);
    tft->print("GAME OVER");
    
    // Draw decorative line
    for(int i = 10; i < tft->width()-10; i+=4) {
      tft->drawPixel(i, 50, ST77XX_RED);
    }
    
    // Display scores with retro styling
    tft->setTextSize(1);
    tft->setTextColor(ST77XX_YELLOW);
    tft->setCursor(20, 65);
    tft->print("SCORE:");
    tft->setTextSize(2);
    tft->setCursor(65, 63);
    tft->print(snake.getScore());
    
    tft->setTextSize(1);
    tft->setTextColor(ST77XX_CYAN);
    tft->setCursor(20, 85);
    tft->print("HIGH SCORE:");
    tft->setTextSize(2);
    tft->setCursor(65, 83);
    tft->print(highScore);
    
    // Blinking restart message
    tft->setTextSize(1);
    if((millis() / 500) % 2) {
      tft->setTextColor(ST77XX_WHITE);
      tft->setCursor(15, 110);
      tft->print("PRESS TO RESTART!");
    }
  }

private:
  void render() {
    // Create current grid state
    uint8_t currentGrid[Snake::GRID_SIZE][Snake::GRID_SIZE] = {0};
    
    // Mark snake positions
    for (int i = 0; i < snake.getLength(); i++) {
      const Point& pos = snake.getPosition(i);
      currentGrid[pos.y][pos.x] = 1;
    }
    
    // Mark food position
    const Point& food = snake.getFood();
    currentGrid[food.y][food.x] = 2;
    
    // Only update changed cells
    for (int y = 0; y < Snake::GRID_SIZE; y++) {
      for (int x = 0; x < Snake::GRID_SIZE; x++) {
        if (currentGrid[y][x] != lastGrid[y][x]) {
          int screenX = offsetX + (x * cellWidth);
          int screenY = offsetY + (y * cellHeight);
          
          // Clear cell
          tft->fillRect(screenX, screenY, cellWidth, cellHeight, ST77XX_BLACK);
          
          // Draw new content if any
          if (currentGrid[y][x] == 1) {
            // Draw snake segment with rounded corners
            tft->fillRoundRect(screenX + 1, screenY + 1,
                            cellWidth - 2, cellHeight - 2,
                            2, ST77XX_GREEN);
          } else if (currentGrid[y][x] == 2) {
            // Draw food as a small circle
            int foodSize = min(cellWidth, cellHeight) - 4;
            int foodX = screenX + (cellWidth - foodSize) / 2;
            int foodY = screenY + (cellHeight - foodSize) / 2;
            tft->fillCircle(foodX + foodSize/2, foodY + foodSize/2,
                         foodSize/2, ST77XX_RED);
          }
          
          lastGrid[y][x] = currentGrid[y][x];
        }
      }
    }

    // Clear and draw score
    // Only update score display if it changed
    int currentScore = snake.getScore();
    if (currentScore != lastScore) {
      tft->fillRect(0, tft->height() - 12, tft->width(), 12, ST77XX_BLACK);
      tft->setTextColor(ST77XX_WHITE);
      tft->setTextSize(1);
      tft->setCursor(2, tft->height() - 10);
      tft->print("Score: ");
      tft->print(currentScore);
      lastScore = currentScore;
    }

    if (snake.isGameOver()) {
      tft->setTextSize(2);
      tft->setCursor(20, tft->height() / 2 - 10);
      tft->setTextColor(ST77XX_RED);
      tft->print("GAME OVER!");
    }
  }

  void drawBorder() {
    tft->drawRect(0, 0, tft->width(), tft->height(), ST77XX_WHITE);
  }

  Adafruit_ST7735* tft;
  InputHandler* input_handler;
  Snake snake;
  int cellWidth;
  int cellHeight;
  int offsetX;
  int offsetY;
  uint8_t lastGrid[Snake::GRID_SIZE][Snake::GRID_SIZE];
  int lastScore = -1;  // Track last score to avoid unnecessary updates
};

#endif