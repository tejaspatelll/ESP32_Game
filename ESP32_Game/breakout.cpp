#include "breakout.h"
#include <Arduino.h>

Breakout::Breakout(Adafruit_ST7735 &tft, uint8_t buttonPin, uint8_t motorPin, int xPin, int yPin) 
    : tft(tft), buttonPin(buttonPin), motorPin(motorPin), xPin(xPin), yPin(yPin), state(INTRO) {}

void Breakout::init() {
    state = INTRO;
    resetGame();
}

void Breakout::update(bool buttonPressed, bool buttonReleased) {
    static unsigned long lastFrameTime = 0;
    const unsigned long frameInterval = 1000 / 60; // 60 FPS
    
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime < frameInterval) {
        return; // Skip frame if not enough time has passed
    }
    lastFrameTime = currentTime;
    
    switch(state) {
        case INTRO:
            if(buttonPressed) {
                state = PLAYING;
            }
            break;
            
        case PLAYING:
            // Paddle movement
            if(analogRead(xPin) < 1600) paddleX = max(0, paddleX - 2);
            if(analogRead(xPin) > 1700) paddleX = min(tft.width() - 20, paddleX + 2);
            
            // Ball movement
            ballX += ballSpeedX;
            ballY += ballSpeedY;
            
            // Ball collision with walls
            if(ballX <= 0 || ballX >= tft.width() - 2) ballSpeedX = -ballSpeedX;
            if(ballY <= 0) ballSpeedY = -ballSpeedY;
            
            // Ball collision with paddle
            if(ballY >= tft.height() - 10 && 
               ballX >= paddleX && ballX <= paddleX + 20) {
                ballSpeedY = -ballSpeedY;
            }
            
            // Ball out of bounds (game over)
            if(ballY >= tft.height()) {
                state = GAME_OVER;
            }
            
            // Brick collision detection would go here
            break;
            
        case GAME_OVER:
            if(buttonPressed) {
                state = INTRO;
                resetGame();
            }
            break;
    }
}

void Breakout::render() {
    static unsigned long lastRenderTime = 0;
    const unsigned long renderInterval = 1000 / 60; // 60 FPS
    
    unsigned long currentTime = millis();
    if (currentTime - lastRenderTime < renderInterval) {
        return; // Skip render if not enough time has passed
    }
    lastRenderTime = currentTime;
    
    switch(state) {
        case INTRO:
            if(!introDrawn) {
                renderIntro();
                introDrawn = true;
            }
            break;
            
        case PLAYING:
            // Clear previous ball position
            tft.fillRect(lastBallX, lastBallY, 2, 2, ST7735_BLACK);
            
            // Clear previous paddle position
            tft.fillRect(lastPaddleX, tft.height() - 8, 20, 1, ST7735_BLACK);
            
            // Draw new paddle position
            tft.fillRect(paddleX, tft.height() - 8, 20, 1, ST7735_WHITE);
            
            // Draw new ball position
            tft.fillRect(ballX, ballY, 2, 2, ST7735_WHITE);
            
            // Store current positions for next frame
            lastPaddleX = paddleX;
            lastBallX = ballX;
            lastBallY = ballY;
            
            // Draw bricks would go here
            break;
            
        case GAME_OVER:
            if(!gameOverDrawn) {
                renderGameOver();
                gameOverDrawn = true;
            }
            break;
    }
}

void Breakout::renderPixel(int x, int y, uint16_t color) {
    tft.drawPixel(x, y, color);
}

void Breakout::renderIntro() {
    tft.fillScreen(ST7735_BLACK);
    
    // 8-bit style title
    tft.drawRect(10, 10, 108, 28, ST7735_WHITE);
    tft.fillRect(12, 12, 104, 24, ST7735_RED);
    tft.setCursor(22, 18);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.print("BREAKOUT");
    
    // 8-bit style button prompt
    tft.fillRect(20, 60, 88, 28, ST7735_BLUE);
    tft.drawRect(18, 58, 92, 32, ST7735_WHITE);
    tft.setCursor(25, 68);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.print("PRESS BUTTON");
}

void Breakout::renderGameOver() {
    tft.fillScreen(ST7735_BLACK);
    
    // 8-bit style game over text
    tft.drawRect(15, 15, 98, 28, ST7735_RED);
    tft.fillRect(17, 17, 94, 24, ST7735_BLACK);
    tft.setCursor(25, 23);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(2);
    tft.print("GAME OVER");
    
    // 8-bit style retry prompt
    tft.fillRect(20, 60, 88, 28, ST7735_GREEN);
    tft.drawRect(18, 58, 92, 32, ST7735_WHITE);
    tft.setCursor(30, 68);
    tft.setTextColor(ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("TRY AGAIN");
}

bool Breakout::isGameOver() {
    return state == GAME_OVER;
}

void Breakout::resetGame() {
    paddleX = tft.width() / 2 - 10;
    ballX = tft.width() / 2;
    ballY = tft.height() / 2;
    ballSpeedX = 1;
    ballSpeedY = -1;
    
    // Initialize bricks
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 10; j++) {
            bricks[i][j] = true;
        }
    }
}