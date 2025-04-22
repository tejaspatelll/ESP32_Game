#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>
#include "inputhandler.h"

struct Point {
  int x, y;
};

class Snake {
public:
  static const int GRID_SIZE = 12; // Adjusted for better fit on 128x128 display
  static const int MAX_LENGTH = 64;
  static const int INITIAL_LENGTH = 3;
  
  Snake(InputHandler* input) : _input(input) {}
  
  void reset() {
    // Initialize snake position (start in middle)
    _length = INITIAL_LENGTH;
    int startX = GRID_SIZE / 2;
    int startY = GRID_SIZE / 2;
    
    // Initialize snake body segments
    for (int i = 0; i < _length; i++) {
      _positions[i].x = startX - i;
      _positions[i].y = startY;
    }
    
    _direction.x = 1;
    _direction.y = 0;
    _score = 0;
    _gameOver = false;
    spawnFood();
  }
  
  void update() {
    if (_gameOver) return;
    
    // Update direction based on input
    if (_input->left && _direction.x != 1) {
      _direction = {-1, 0};
    } else if (_input->right && _direction.x != -1) {
      _direction = {1, 0};
    } else if (_input->up && _direction.y != 1) {
      _direction = {0, -1};
    } else if (_input->down && _direction.y != -1) {
      _direction = {0, 1};
    }
    
    // Move snake
    Point newHead = {
      (_positions[0].x + _direction.x + GRID_SIZE) % GRID_SIZE,
      (_positions[0].y + _direction.y + GRID_SIZE) % GRID_SIZE
    };
    
    // Check collision with self (skip head)
    for (int i = 1; i < _length; i++) {
      if (newHead.x == _positions[i].x && newHead.y == _positions[i].y) {
        _gameOver = true;
        return;
      }
    }
    
    // Check food collision first
    bool ateFood = (newHead.x == _food.x && newHead.y == _food.y);
    
    // Move body
    if (ateFood) {
      // When eating, shift all segments right and add new head
      for (int i = _length; i > 0; i--) {
        _positions[i] = _positions[i - 1];
      }
      _positions[0] = newHead;
      _length++;
      _score += 10;
      spawnFood();
    } else {
      // Normal movement - shift all segments except head
      for (int i = _length - 1; i > 0; i--) {
        _positions[i] = _positions[i - 1];
      }
      _positions[0] = newHead;
    }
  }
  
  void render() {
    // Clear screen
    Serial.println("\033[2J\033[H");
    
    // Draw border and game area
    for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++) {
        bool isSnake = false;
        for (int i = 0; i < _length; i++) {
          if (_positions[i].x == x && _positions[i].y == y) {
            isSnake = true;
            break;
          }
        }
        
        if (x == _food.x && y == _food.y) {
          Serial.print("O");
        } else if (isSnake) {
          Serial.print("#");
        } else {
          Serial.print(".");
        }
      }
      Serial.println();
    }
    
    Serial.print("Score: ");
    Serial.println(_score);
    
    if (_gameOver) {
      Serial.println("Game Over!");
    }
  }
  
  bool isGameOver() const { return _gameOver; }
  int getScore() const { return _score; }
  
  // Getter methods for rendering
  const Point& getFood() const { return _food; }
  int getLength() const { return _length; }
  const Point& getPosition(int index) const { return _positions[index]; }
  
private:
  
  void spawnFood() {
    do {
      _food.x = random(GRID_SIZE);
      _food.y = random(GRID_SIZE);
      
      bool onSnake = false;
      for (int i = 0; i < _length; i++) {
        if (_food.x == _positions[i].x && _food.y == _positions[i].y) {
          onSnake = true;
          break;
        }
      }
      
      if (!onSnake) break;
    } while (true);
  }
  
  InputHandler* _input;
  Point _positions[MAX_LENGTH];
  Point _direction;
  Point _food;
  int _length;
  int _score;
  bool _gameOver;
};

#endif