# ESP32 Joystick Display Project

## Project Description
This project implements a game menu system for an ESP32 microcontroller with joystick input and ST7735 display. It currently supports Space Invaders and Snake games with potential for additional games.

## Hardware Requirements
- ESP32 microcontroller
- ST7735 TFT display
- Joystick module
- Breadboard and jumper wires

## Installation
1. Clone this repository
2. Open the project in Arduino IDE
3. Install required libraries:
   - Adafruit ST7735 library
   - Adafruit GFX library
   - EEPROM library (for Snake game high score storage)
4. Upload the sketch to your ESP32

## Usage
1. Power on the device
2. Use the joystick to navigate the menu:
   - Up/Down: Navigate menu items
   - Button press: Select game
3. Selected game will launch automatically

## Game Controls
### Space Invaders
- Left/Right: Move spaceship
- Button: Fire weapon

### Snake Game
- Left/Right/Up/Down: Change snake direction
- Button: Start game or restart after game over

## Features
- High score saving for Snake game using EEPROM
- Retro-style graphics for both games
- Responsive controls with joystick input

## Adding New Games
1. Create two new files for your game:
   - `yourgame.h` - Header file with class declaration (see breakout.h for example)
   - `yourgame.cpp` - Implementation file with game logic (see breakout.cpp for example)
2. Include your game header in gamemenu.h
3. Update the menuItems array in GameMenu::init() to include your new game
4. Implement core game functions in your class:
   - `init()` - Initialize game state
   - `update()` - Handle input and game logic
   - `render()` - Draw game graphics
   - `isGameOver()` - Check game end condition
5. Follow the existing pattern for:
   - Input handling (joystick/button)
   - Display rendering (ST7735 library)
   - Game state management (INTRO/PLAYING/GAME_OVER)

## Future Game Ideas
- Racing Game: Top-down racing game avoiding obstacles
- Platformer: Simple platforming game with jumping
- Breakout: Control a paddle to bounce a ball and break bricks
- Tetris: Classic block-pushing puzzle game
- Pong: Simple two-player paddle game