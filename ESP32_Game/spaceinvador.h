#ifndef SPACEINVADOR_H
#define SPACEINVADOR_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <EEPROM.h>

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

// Game objects
struct Alien {
  int x, y;
  boolean alive;
};

struct Bullet {
  int x, y;
  boolean active;
};

struct Shield {
  int x, y;
  int health;
};

// Player bitmap (11x8)
static const unsigned char PROGMEM playerBitmap[] = {
  0b00001000, 0b00000000,
  0b00011100, 0b00000000,
  0b00111110, 0b00000000,
  0b01111111, 0b00000000,
  0b11111111, 0b10000000,
  0b11111111, 0b10000000,
  0b11111111, 0b10000000,
  0b11111111, 0b10000000
};

// Alien bitmap (8x8)
static const unsigned char PROGMEM alienBitmap[] = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11011011,
  0b11111111,
  0b00100100,
  0b01011010,
  0b10000001
};

class SpaceInvador {
public:
  // Game states
  enum GameState {
    START,
    PLAYING,
    GAME_OVER
  };
  
  SpaceInvador(Adafruit_ST7735 &display, int buttonPin, int vibrationPin, int xPin, int yPin) : 
    tft(display), buttonPin(buttonPin), vibrationPin(vibrationPin), xPin(xPin), yPin(yPin) {
    // Initialize game variables
    currentState = START;
    gameOverScreenShown = false;
    buttonWasPressed = false;
    playerX = 0;
    oldPlayerX = 0;
    score = 0;
    lives = 3;
    lastShot = 0;
    lastAlienMove = 0;
    lastAlienShot = 0;
    alienDirection = 1;
    highScore = 0;
    
    // Read high score from EEPROM
    highScore = EEPROM.read(highScoreAddress) | (EEPROM.read(highScoreAddress + 1) << 8);
  }
  
  void init() {
    // Initialize player position
    playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
    oldPlayerX = playerX;
    
    // Initialize aliens
    for (int row = 0; row < ALIEN_ROWS; row++) {
      for (int col = 0; col < ALIEN_COLS; col++) {
        int index = row * ALIEN_COLS + col;
        aliens[index].x = 10 + col * ALIEN_SPACING_X;
        aliens[index].y = 20 + row * ALIEN_SPACING_Y;
        aliens[index].alive = true;
      }
    }
    
    // Initialize shields
    for (int i = 0; i < SHIELD_COUNT; i++) {
      shields[i].x = 20 + i * (SCREEN_WIDTH / SHIELD_COUNT);
      shields[i].y = SCREEN_HEIGHT - 30;
      shields[i].health = 3;
    }
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      bullets[i].active = false;
    }
    
    // Initialize alien bullets
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
      alienBullets[i].active = false;
    }
    
    // Reset game state
    score = 0;
    lives = 3;
    currentState = PLAYING;
    
    // Clear screen
    tft.fillScreen(BLACK);
    
    // Draw initial game elements
    drawScore();
    drawLives();
    drawPlayer();
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      drawAlien(i);
    }
    
    for (int i = 0; i < SHIELD_COUNT; i++) {
      drawShield(i);
    }
  }
  
  // Main update function to be called from the main loop
  void update(bool buttonPressed, bool buttonReleased) {
    switch (currentState) {
      case START:
        handleStartState(buttonPressed);
        break;
      case PLAYING:
        handlePlayingState(buttonPressed);
        break;
      case GAME_OVER:
        handleGameOverState(buttonPressed, buttonReleased);
        break;
    }
  }
  
  // Get current game state
  GameState getState() {
    return currentState;
  }
  
  // Set game state
  void setState(GameState state) {
    currentState = state;
    
    // Reset state-specific variables
    if (state == GAME_OVER) {
      gameOverScreenShown = false;
    }
  }
  
  // Handle start screen state
  void handleStartState(bool buttonPressed) {
    if (!startScreenShown) {
      showStartScreen();
      startScreenShown = true;
    }
    
    if (buttonPressed) {
      currentState = PLAYING;
      startScreenShown = false;
      init(); // Initialize game
    }
  }
  
  // Handle playing state
  void handlePlayingState(bool buttonPressed) {
    static unsigned long lastFrameTime = 0;
    const unsigned long frameInterval = 1000 / 60; // 60 FPS
    
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime < frameInterval) {
      return; // Skip frame if not enough time has passed
    }
    lastFrameTime = currentTime;
    
    // Read joystick for player movement
    int xValue = analogRead(xPin);
    
    // Store old position for erasing
    oldPlayerX = playerX;
    
    // Move player based on joystick input with deadzone
    if (xValue < 1600) {
      playerX = max(0, playerX - PLAYER_SPEED);
    } else if (xValue > 1700) {
      playerX = min(SCREEN_WIDTH - PLAYER_WIDTH, playerX + PLAYER_SPEED);
    }
    
    // Only redraw player if position changed
    if (oldPlayerX != playerX) {
      erasePlayer();
      drawPlayer();
    }
    
    // Shoot when button pressed (with debounce)
    if (buttonPressed && millis() - lastShot > 200) {
      firePlayerBullet();
      lastShot = millis();
    }
    
    // Update bullets
    updateBullets();
    
    // Update alien bullets
    updateAlienBullets();
    
    // Move aliens periodically
    if (millis() - lastAlienMove > 500) {
      moveAliens();
      lastAlienMove = millis();
      
      // Randomly fire alien bullets
      if (random(100) < 30 && millis() - lastAlienShot > 800) {
        fireAlienBullet();
        lastAlienShot = millis();
      }
    }
    
    // Check if all aliens are dead
    if (aliensAllDead()) {
      levelComplete();
    }
  }
  
  // Handle game over state
  void handleGameOverState(bool buttonPressed, bool buttonReleased) {
    if (!gameOverScreenShown) {
      gameOverScreen();
      gameOverScreenShown = true;
    }
    
    if (buttonPressed && !buttonWasPressed) {
      currentState = START;
      gameOverScreenShown = false;
      buttonWasPressed = true;
    } else if (buttonReleased) {
      buttonWasPressed = false;
    }
  }
  
  void draw() {
    static unsigned long lastRenderTime = 0;
    const unsigned long renderInterval = 1000 / 60; // 60 FPS
    
    unsigned long currentTime = millis();
    if (currentTime - lastRenderTime < renderInterval) {
      return; // Skip render if not enough time has passed
    }
    lastRenderTime = currentTime;
    
    if (currentState != PLAYING) return;
    
    // Draw all game elements
    drawScore();
    drawLives();
    
    for (int i = 0; i < SHIELD_COUNT; i++) {
      if (shields[i].health > 0) {
        drawShield(i);
      }
    }
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        drawAlien(i);
      }
    }
    
    drawPlayer();
    
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        tft.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, GREEN);
      }
    }
    
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
      if (alienBullets[i].active) {
        tft.fillRect(alienBullets[i].x, alienBullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, RED);
      }
    }
  }
  
  bool isRunning() { return currentState == PLAYING; }
  void start() { currentState = PLAYING; initGame(); }
  void stop() { currentState = GAME_OVER; }
  
// **Screen Display Functions**
void showStartScreen() {
    tft.fillScreen(BLACK);
    
    // Calculate center positions
    int titleWidth = 12 * 6 * 2; // 12 chars * 6px * text size 2
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(titleX, 20);
    tft.print("SPACE INVADERS");
    
    // High score centered
    String scoreText = "High Score: " + String(highScore);
    int scoreWidth = scoreText.length() * 6; // 6px per char at size 1
    int scoreX = (SCREEN_WIDTH - scoreWidth) / 2;
    
    tft.setTextSize(1);
    tft.setCursor(scoreX, 50);
    tft.print(scoreText);
    
    // Button prompt centered
    int buttonWidth = 19 * 6; // "Press button to start" is 19 chars
    int buttonX = (SCREEN_WIDTH - buttonWidth) / 2;
    
    tft.setCursor(buttonX, 70);
    tft.print("Press button to start");
    
    // Add decorative lines
    tft.drawFastHLine(0, 40, SCREEN_WIDTH, WHITE);
    tft.drawFastHLine(0, 60, SCREEN_WIDTH, WHITE);
  }
  
  void gameOverScreen() {
    tft.fillScreen(BLACK);
    
    // Center "GAME OVER" text
    int gameOverWidth = 9 * 6 * 2; // 9 chars * 6px * text size 2
    int gameOverX = (SCREEN_WIDTH - gameOverWidth) / 2;
    
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.setCursor(gameOverX, 30);
    tft.print("GAME OVER");
    
    // Center score text
    String scoreText = "Score: " + String(score);
    int scoreWidth = scoreText.length() * 6;
    int scoreX = (SCREEN_WIDTH - scoreWidth) / 2;
    
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.setCursor(scoreX, 60);
    tft.print(scoreText);
    
    // Center high score text
    String highScoreText = "High Score: " + String(highScore);
    int highScoreWidth = highScoreText.length() * 6;
    int highScoreX = (SCREEN_WIDTH - highScoreWidth) / 2;
    
    tft.setCursor(highScoreX, 80);
    if (score > highScore) {
      highScore = score;
      EEPROM.write(highScoreAddress, highScore & 0xFF);
      EEPROM.write(highScoreAddress + 1, (highScore >> 8) & 0xFF);
      tft.print(highScoreText);
      
      // Center "NEW HIGH SCORE!" text
      int newHighWidth = 15 * 6; // 15 chars
      int newHighX = (SCREEN_WIDTH - newHighWidth) / 2;
      
      tft.setTextColor(GREEN);
      tft.setCursor(newHighX, 100);
      tft.print("NEW HIGH SCORE!");
    } else {
      tft.print(highScoreText);
    }
    
    // Center restart prompt
    int restartWidth = 22 * 6; // "Press button to restart" is 22 chars
    int restartX = (SCREEN_WIDTH - restartWidth) / 2;
    
    tft.setTextColor(WHITE);
    tft.setCursor(restartX, 120);
    tft.print("Press button to restart");
    
    // Add decorative border
    tft.drawRect(5, 5, SCREEN_WIDTH-10, SCREEN_HEIGHT-10, WHITE);
    
    // Vibration feedback
    digitalWrite(vibrationPin, HIGH);
    delay(500);
    digitalWrite(vibrationPin, LOW);
  }
  
  // **Initialization Function**
  void initGame() {
    // Initialize player
    playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
    oldPlayerX = playerX;
    
    // Initialize aliens
    for (int row = 0; row < ALIEN_ROWS; row++) {
      for (int col = 0; col < ALIEN_COLS; col++) {
        int index = row * ALIEN_COLS + col;
        aliens[index].x = 10 + col * ALIEN_SPACING_X;
        aliens[index].y = 15 + row * ALIEN_SPACING_Y;
        aliens[index].alive = true;
      }
    }
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      bullets[i].active = false;
    }
    
    // Initialize alien bullets
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
      alienBullets[i].active = false;
    }
    
    // Initialize shields
    for (int i = 0; i < SHIELD_COUNT; i++) {
      shields[i].x = 20 + i * ((SCREEN_WIDTH - 40) / (SHIELD_COUNT - 1));
      shields[i].y = SCREEN_HEIGHT - 30;
      shields[i].health = 3;
    }
    
    // Reset game state
    score = 0;
    lives = 3;
    alienDirection = 1;
    
    // Draw initial screen
    tft.fillScreen(BLACK);
    drawScore();
    drawLives();
    
    // Draw shields
    for (int i = 0; i < SHIELD_COUNT; i++) {
      drawShield(i);
    }
    
    // Draw aliens
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        drawAlien(i);
      }
    }
    
    // Draw player
    drawPlayer();
  }
  
  // **Drawing Functions**
  void drawPlayer() {
    tft.drawBitmap(playerX, SCREEN_HEIGHT - PLAYER_HEIGHT - 10, playerBitmap, PLAYER_WIDTH, PLAYER_HEIGHT, GREEN);
  }
  
  void erasePlayer() {
    tft.fillRect(oldPlayerX, SCREEN_HEIGHT - PLAYER_HEIGHT - 10, PLAYER_WIDTH, PLAYER_HEIGHT, BLACK);
  }
  
  void drawAlien(int index) {
    tft.drawBitmap(aliens[index].x, aliens[index].y, alienBitmap, ALIEN_WIDTH, ALIEN_HEIGHT, WHITE);
  }
  
  void eraseAlien(int index) {
    tft.fillRect(aliens[index].x, aliens[index].y, ALIEN_WIDTH, ALIEN_HEIGHT, BLACK);
  }
  
  void drawShield(int index) {
    uint16_t color;
    switch (shields[index].health) {
      case 3: color = GREEN; break;
      case 2: color = 0xFFE0; break; // Yellow
      case 1: color = RED; break;
      default: color = BLACK; break;
    }
    tft.fillRect(shields[index].x - SHIELD_WIDTH/2, shields[index].y, SHIELD_WIDTH, SHIELD_HEIGHT, color);
  }
  
  void drawScore() {
    tft.fillRect(0, 0, 60, 8, BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Score:");
    tft.print(score);
  }
  
  void drawLives() {
    tft.fillRect(SCREEN_WIDTH - 40, 0, 40, 8, BLACK);
    tft.setCursor(SCREEN_WIDTH - 40, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Lives:");
    tft.print(lives);
  }
  
  // **Game Logic Functions**
  void firePlayerBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = playerX + PLAYER_WIDTH/2 - BULLET_WIDTH/2;
        bullets[i].y = SCREEN_HEIGHT - PLAYER_HEIGHT - 10;
        bullets[i].active = true;
        
        // Vibration feedback
        digitalWrite(vibrationPin, HIGH);
        delay(50);
        digitalWrite(vibrationPin, LOW);
        break;
      }
    }
  }
  
  void fireAlienBullet() {
    int aliveAliens[ALIEN_ROWS * ALIEN_COLS];
    int aliveCount = 0;
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        aliveAliens[aliveCount++] = i;
      }
    }
    
    if (aliveCount > 0) {
      int alienIndex = aliveAliens[random(aliveCount)];
      for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (!alienBullets[i].active) {
          alienBullets[i].x = aliens[alienIndex].x + ALIEN_WIDTH/2 - BULLET_WIDTH/2;
          alienBullets[i].y = aliens[alienIndex].y + ALIEN_HEIGHT;
          alienBullets[i].active = true;
          break;
        }
      }
    }
  }
  
  void updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        tft.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);
        bullets[i].y -= BULLET_SPEED;
        
        if (bullets[i].y < 0) {
          bullets[i].active = false;
          continue;
        }
        
        boolean hit = false;
        for (int j = 0; j < ALIEN_ROWS * ALIEN_COLS && !hit; j++) {
          if (aliens[j].alive && collisionCheck(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                                                aliens[j].x, aliens[j].y, ALIEN_WIDTH, ALIEN_HEIGHT)) {
            eraseAlien(j);
            aliens[j].alive = false;
            bullets[i].active = false;
            hit = true;
            score += 10;
            drawScore();
            
            digitalWrite(vibrationPin, HIGH);
            delay(30);
            digitalWrite(vibrationPin, LOW);
          }
        }
        
        for (int j = 0; j < SHIELD_COUNT && !hit; j++) {
          if (shields[j].health > 0 && collisionCheck(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                                                      shields[j].x - SHIELD_WIDTH/2, shields[j].y, SHIELD_WIDTH, SHIELD_HEIGHT)) {
            bullets[i].active = false;
            hit = true;
            shields[j].health--;
            drawShield(j);
          }
        }
        
        if (bullets[i].active) {
          tft.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, GREEN);
        }
      }
    }
  }
  
  void updateAlienBullets() {
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
      if (alienBullets[i].active) {
        tft.fillRect(alienBullets[i].x, alienBullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, BLACK);
        alienBullets[i].y += BULLET_SPEED;
        
        if (alienBullets[i].y > SCREEN_HEIGHT) {
          alienBullets[i].active = false;
          continue;
        }
        
        if (collisionCheck(alienBullets[i].x, alienBullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                           playerX, SCREEN_HEIGHT - PLAYER_HEIGHT - 10, PLAYER_WIDTH, PLAYER_HEIGHT)) {
          alienBullets[i].active = false;
          playerHit();
          continue;
        }
        
        boolean hit = false;
        for (int j = 0; j < SHIELD_COUNT && !hit; j++) {
          if (shields[j].health > 0 && collisionCheck(alienBullets[i].x, alienBullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                                                      shields[j].x - SHIELD_WIDTH/2, shields[j].y, SHIELD_WIDTH, SHIELD_HEIGHT)) {
            alienBullets[i].active = false;
            hit = true;
            shields[j].health--;
            drawShield(j);
          }
        }
        
        if (alienBullets[i].active) {
          tft.fillRect(alienBullets[i].x, alienBullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, RED);
        }
      }
    }
  }
  
  void moveAliens() {
    boolean changeDirection = false;
    int leftX = SCREEN_WIDTH;
    int rightX = 0;
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        leftX = min(leftX, aliens[i].x);
        rightX = max(rightX, aliens[i].x + ALIEN_WIDTH);
      }
    }
    
    if ((rightX >= SCREEN_WIDTH - 2 && alienDirection > 0) || (leftX <= 2 && alienDirection < 0)) {
      changeDirection = true;
    }
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        eraseAlien(i);
        
        if (changeDirection) {
          aliens[i].y += 8;
          alienDirection *= -1;
        } else {
          aliens[i].x += alienDirection;
        }
        
        drawAlien(i);
        
        if (aliens[i].y + ALIEN_HEIGHT >= SCREEN_HEIGHT - PLAYER_HEIGHT - 10) {
          currentState = GAME_OVER;
          return;
        }
      }
    }
  }
  
  bool collisionCheck(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
  }
  
  void playerHit() {
    lives--;
    drawLives();
    
    digitalWrite(vibrationPin, HIGH);
    delay(200);
    digitalWrite(vibrationPin, LOW);
    
    if (lives <= 0) {
      currentState = GAME_OVER;
    }
  }
  
  bool aliensAllDead() {
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      if (aliens[i].alive) {
        return false;
      }
    }
    return true;
  }
  
  void levelComplete() {
    tft.fillRect(10, 50, SCREEN_WIDTH - 20, 20, BLACK);
    tft.setCursor(20, 55);
    tft.setTextColor(GREEN);
    tft.setTextSize(1);
    tft.print("LEVEL COMPLETE!");
    
    digitalWrite(vibrationPin, HIGH);
    delay(100);
    digitalWrite(vibrationPin, LOW);
    delay(100);
    digitalWrite(vibrationPin, HIGH);
    delay(100);
    digitalWrite(vibrationPin, LOW);
    
    delay(2000);
    
    // Reset aliens but keep score and lives
    for (int row = 0; row < ALIEN_ROWS; row++) {
      for (int col = 0; col < ALIEN_COLS; col++) {
        int index = row * ALIEN_COLS + col;
        aliens[index].x = 10 + col * ALIEN_SPACING_X;
        aliens[index].y = 15 + row * ALIEN_SPACING_Y;
        aliens[index].alive = true;
      }
    }
    
    // Redraw screen
    tft.fillScreen(BLACK);
    drawScore();
    drawLives();
    
    for (int i = 0; i < SHIELD_COUNT; i++) {
      shields[i].health = 3;
      drawShield(i);
    }
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
      drawAlien(i);
    }
    
    drawPlayer();
  }


private:
  // Game variables
  Adafruit_ST7735 &tft;
  int buttonPin;
  int vibrationPin;
  int xPin;
  int yPin;
  GameState currentState;
  
  int playerX, oldPlayerX;
  int score;
  int lives;
  unsigned long lastShot;
  unsigned long lastAlienMove;
  unsigned long lastAlienShot;
  int alienDirection;
  boolean startScreenShown;
  boolean gameOverScreenShown;
  boolean buttonWasPressed;
  int highScore;
  const int highScoreAddress = 0;

  Alien aliens[ALIEN_ROWS * ALIEN_COLS];
  Bullet bullets[MAX_BULLETS];
  Bullet alienBullets[MAX_ALIEN_BULLETS];
  Shield shields[SHIELD_COUNT];
};

#endif