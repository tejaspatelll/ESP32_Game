#ifndef FLAPPYBIRD_H
#define FLAPPYBIRD_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Game constants
#define BIRD_WIDTH 8
#define BIRD_HEIGHT 8
#define PIPE_WIDTH 15
#define PIPE_GAP 35
#define GRAVITY 0.6
#define JUMP_FORCE 5.0
#define MAX_PIPES 3
#define FRAME_TIME 16  // Target ~60 FPS (1000ms/60)
#define BUFFER_HEIGHT 20  // Height of update buffer regions

// Colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define BLUE 0x001F
#define YELLOW 0xFFE0
#define RED 0xF800

// Game objects
struct Bird {
  float x, y;
  float prevX, prevY;
  float velocity;
  bool needsUpdate;
};

struct Pipe {
  int x, prevX;
  int gapY;
  bool passed;
  bool needsUpdate;
  int prevTopHeight;
  int prevBottomY;
};

// Bird sprite (8x8)
static const uint16_t PROGMEM birdSprite[] = {
  BLACK, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, BLACK, BLACK,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, BLACK,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW,
  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, BLACK,
  BLACK, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, BLACK, BLACK,
  BLACK, BLACK, YELLOW, BLACK, BLACK, BLACK, BLACK, BLACK
};

class FlappyBird {
public:
  enum GameState {
    START,
    PLAYING,
    GAME_OVER
  };
  
  FlappyBird(Adafruit_ST7735 &display, int buttonPin, int vibrationPin, int xPin, int yPin) : 
    tft(display), buttonPin(buttonPin), vibrationPin(vibrationPin), xPin(xPin), yPin(yPin) {
    currentState = START;
    gameOverScreenShown = false;
    buttonWasPressed = false;
    score = 0;
    highScore = 0;
    lastFrameTime = 0;
  }
  
  void init() {
    currentState = START;
    gameOverScreenShown = false;
    score = 0;
    prevScore = 0;
    
    bird.x = 30;
    bird.y = SCREEN_HEIGHT / 2;
    bird.velocity = 0;
    
    for (int i = 0; i < MAX_PIPES; i++) {
      pipes[i].x = SCREEN_WIDTH + (i * (SCREEN_WIDTH / 2));
      pipes[i].gapY = random(PIPE_GAP, SCREEN_HEIGHT - PIPE_GAP);
      pipes[i].passed = false;
      pipes[i].needsUpdate = true;
    }
    
    tft.fillScreen(BLACK);
    drawStartScreen();
  }
  
  void update(bool buttonPressed, bool buttonReleased) {
    static unsigned long lastFrameTime = 0;
    const unsigned long frameInterval = 1000 / 60; // 60 FPS
    
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime < frameInterval) {
      return; // Skip frame if not enough time has passed
    }
    lastFrameTime = currentTime;

    switch (currentState) {
      case START:
        handleStartState(buttonPressed);
        break;
      case PLAYING:
        handlePlayingState(buttonPressed);
        break;
      case GAME_OVER:
        handleGameOverState(buttonPressed);
        break;
    }
  }
  
  GameState getState() { return currentState; }
  
private:
  Adafruit_ST7735 &tft;
  int buttonPin, vibrationPin, xPin, yPin;
  GameState currentState;
  bool gameOverScreenShown, buttonWasPressed;
  Bird bird;
  Pipe pipes[MAX_PIPES];
  int score, prevScore, highScore;
  unsigned long lastFrameTime;
  
  void handleStartState(bool buttonPressed) {
    if (buttonPressed) {
      currentState = PLAYING;
      tft.fillScreen(BLACK);
    }
  }
  
  void handlePlayingState(bool buttonPressed) {
    // Physics update
    bird.velocity += GRAVITY;
    if (buttonPressed && !buttonWasPressed) {
      bird.velocity = -JUMP_FORCE;
      buttonWasPressed = true;
    } else if (!buttonPressed) {
      buttonWasPressed = false;
    }
    
    float newY = bird.y + bird.velocity;
    if (newY < 0) {
      newY = 0;
      bird.velocity = 0;
    } else if (newY > SCREEN_HEIGHT - BIRD_HEIGHT) {
      newY = SCREEN_HEIGHT - BIRD_HEIGHT;
      gameOver();
      return;
    }
    
    // Store previous position and update flag if position changed
    bird.prevX = bird.x;
    bird.prevY = bird.y;
    bird.y = newY;
    bird.needsUpdate = (bird.prevY != bird.y);
    
    // Only update if position changed
    if (bird.needsUpdate) {
      clearBirdRegion();
      drawBird();
    }
    
    // Update and draw pipes
    updatePipes();
    
    // Update score if needed
    if (prevScore != score) {
      drawScore();
      prevScore = score;
    }
  }
  
  void clearBirdRegion() {
    if (bird.needsUpdate) {
      // Only clear the previous position
      tft.fillRect(bird.prevX, bird.prevY, BIRD_WIDTH, BIRD_HEIGHT, BLACK);
    }
  }
  
  void drawBird() {
    tft.drawRGBBitmap(bird.x, bird.y, birdSprite, BIRD_WIDTH, BIRD_HEIGHT);
  }
  
  void updatePipes() {
    for (int i = 0; i < MAX_PIPES; i++) {
      // Store previous position
      pipes[i].prevX = pipes[i].x;
      pipes[i].prevTopHeight = pipes[i].gapY - PIPE_GAP/2;
      pipes[i].prevBottomY = pipes[i].gapY + PIPE_GAP/2;
      
      // Move pipe
      pipes[i].x -= 2;
      
      // Only mark for update if position changed
      pipes[i].needsUpdate = (pipes[i].prevX != pipes[i].x);
      
      if (pipes[i].needsUpdate) {
        // Clear previous pipe edges
        clearPipeEdges(i);
        // Draw new pipe edges
        drawPipe(i);
        pipes[i].needsUpdate = false;
      }
      
      // Reset pipe if off screen with proper spacing
      if (pipes[i].x < -PIPE_WIDTH) {
        // Clear the entire pipe before resetting
        tft.fillRect(pipes[i].x, 0, PIPE_WIDTH, SCREEN_HEIGHT, BLACK);
        
        // Find the pipe with maximum x position to ensure proper spacing
        int maxX = 0;
        for (int j = 0; j < MAX_PIPES; j++) {
          if (pipes[j].x > maxX) maxX = pipes[j].x;
        }
        
        // Place new pipe with minimum spacing of SCREEN_WIDTH/2
        pipes[i].x = max(maxX + SCREEN_WIDTH/2, SCREEN_WIDTH);
        pipes[i].gapY = random(PIPE_GAP, SCREEN_HEIGHT - PIPE_GAP);
        pipes[i].passed = false;
      }
      
      // Check collision and scoring
      if (checkCollision(i)) {
        gameOver();
        return;
      }
      if (!pipes[i].passed && bird.x > pipes[i].x + PIPE_WIDTH) {
        pipes[i].passed = true;
        score++;
      }
    }
  }
  
  void clearPipeEdges(int index) {
    // Clear all edges of the pipe at previous position
    // Clear left edge
    tft.drawFastVLine(pipes[index].prevX, 0,
                    pipes[index].prevTopHeight, BLACK); // Top pipe left
    tft.drawFastVLine(pipes[index].prevX, pipes[index].prevBottomY,
                    SCREEN_HEIGHT - pipes[index].prevBottomY, BLACK); // Bottom pipe left
    
    // Clear right edge
    tft.drawFastVLine(pipes[index].prevX + PIPE_WIDTH - 1, 0,
                    pipes[index].prevTopHeight, BLACK); // Top pipe right
    tft.drawFastVLine(pipes[index].prevX + PIPE_WIDTH - 1, pipes[index].prevBottomY,
                    SCREEN_HEIGHT - pipes[index].prevBottomY, BLACK); // Bottom pipe right
    
    // Clear top and bottom edges
    tft.drawFastHLine(pipes[index].prevX, pipes[index].prevTopHeight - 1, PIPE_WIDTH, BLACK);
    tft.drawFastHLine(pipes[index].prevX, pipes[index].prevBottomY, PIPE_WIDTH, BLACK);
  }
  
  bool checkCollision(int pipeIndex) {
    if (bird.x + BIRD_WIDTH > pipes[pipeIndex].x && 
        bird.x < pipes[pipeIndex].x + PIPE_WIDTH) {
      if (bird.y < pipes[pipeIndex].gapY - PIPE_GAP/2 || 
          bird.y + BIRD_HEIGHT > pipes[pipeIndex].gapY + PIPE_GAP/2) {
        return true;
      }
    }
    return false;
  }
  
  void drawPipe(int index) {
    // Draw 1px edges of the pipe
    // Draw leading and trailing edges of top pipe
    tft.drawFastVLine(pipes[index].x, 0,
                    pipes[index].gapY - PIPE_GAP/2, GREEN); // Leading edge
    tft.drawFastVLine(pipes[index].x + PIPE_WIDTH - 1, 0,
                    pipes[index].gapY - PIPE_GAP/2, GREEN); // Trailing edge
    
    // Draw leading and trailing edges of bottom pipe
    tft.drawFastVLine(pipes[index].x, pipes[index].gapY + PIPE_GAP/2,
                    SCREEN_HEIGHT - (pipes[index].gapY + PIPE_GAP/2), GREEN); // Leading edge
    tft.drawFastVLine(pipes[index].x + PIPE_WIDTH - 1, pipes[index].gapY + PIPE_GAP/2,
                    SCREEN_HEIGHT - (pipes[index].gapY + PIPE_GAP/2), GREEN); // Trailing edge
    
    // Draw top and bottom edges for better visibility
    tft.drawFastHLine(pipes[index].x, pipes[index].gapY - PIPE_GAP/2 - 1, PIPE_WIDTH, GREEN);
    tft.drawFastHLine(pipes[index].x, pipes[index].gapY + PIPE_GAP/2, PIPE_WIDTH, GREEN);
  }
  
  void gameOver() {
    currentState = GAME_OVER;
    digitalWrite(vibrationPin, HIGH);
    delay(200);
    digitalWrite(vibrationPin, LOW);
  }
  
  void handleGameOverState(bool buttonPressed) {
    if (!gameOverScreenShown) {
      drawGameOverScreen();
      gameOverScreenShown = true;
      if (score > highScore) highScore = score;
    }
    
    if (buttonPressed && !buttonWasPressed) {
      buttonWasPressed = true;
    } else if (!buttonPressed && buttonWasPressed) {
      buttonWasPressed = false;
      init();
    }
  }
  
  void drawScore() {
    tft.fillRect(5, 5, 50, 10, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("Score: ");
    tft.print(score);
  }
  
  void drawStartScreen() {
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.setCursor(20, 40);
    tft.print("FLAPPY BIRD");
    tft.setCursor(15, 60);
    tft.print("Press button");
    tft.setCursor(25, 70);
    tft.print("to start");
    drawBird();
  }
  
  void drawGameOverScreen() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.setCursor(30, 40);
    tft.print("GAME OVER");
    tft.setCursor(30, 60);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(30, 70);
    tft.print("Best: ");
    tft.print(highScore);
    tft.setCursor(15, 90);
    tft.print("Press button");
    tft.setCursor(15, 100);
    tft.print("to play again");
  }
};

#endif