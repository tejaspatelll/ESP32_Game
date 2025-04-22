#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "inputhandler.h"

class Breakout {
public:
    enum GameState { INTRO, PLAYING, GAME_OVER };
    
    Breakout(Adafruit_ST7735 &tft, uint8_t buttonPin, uint8_t motorPin, int xPin, int yPin);
    void init();
    void update(bool buttonPressed, bool buttonReleased);
    void render();
    bool isGameOver();
    
private:
    Adafruit_ST7735 &tft;
    uint8_t buttonPin;
    uint8_t motorPin;
    int xPin;
    int yPin;
    GameState state;
    
    // Game variables
    int paddleX;
    int ballX, ballY;
    int ballSpeedX, ballSpeedY;
    bool bricks[5][10]; // 5 rows, 10 columns of bricks
    
    // Rendering state variables
    bool introDrawn = false;
    bool gameOverDrawn = false;
    int lastBallX = 0;
    int lastBallY = 0;
    int lastPaddleX = 0;
    
    void renderPixel(int x, int y, uint16_t color);
    void renderIntro();
    void renderGameOver();
    void resetGame();
};

#endif