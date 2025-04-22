#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "spaceinvador.h"
#include "snake.h"
#include "breakout.h"
#include "inputhandler.h"

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0
#define WHITE 0xFFFF
#define YELLOW 0xFFE0

#define MAX_MENU_ITEMS 10 // Maximum number of menu items
#define MENU_ITEM_HEIGHT 25 // Increased height for better spacing
#define MENU_TEXT_SIZE 1
#define MENU_SELECTOR_WIDTH 8 // Wider selector for better visibility
#define TITLE_TEXT_SIZE 2
#define MENU_ITEM_PADDING 12 // Padding for menu items
#define TITLE_HEIGHT 30 // Height for the title section
#define MENU_START_Y (TITLE_HEIGHT + 10) // Starting Y position for menu items

// Colors
#define MENU_BG BLACK
#define TITLE_COLOR BLUE
#define SELECTED_COLOR YELLOW
#define UNSELECTED_COLOR WHITE
#define SELECTOR_COLOR GREEN

enum GameType { SPACE_INVADOR, SNAKE, BREAKOUT };

class GameMenu {
public:
  GameMenu(Adafruit_ST7735 &display, InputHandler &inputHandler);
  void init();
  void draw();
  
  int selectedItem = 0;
  bool shouldLaunchGame = false;
  int currentGameIndex = -1;
  
private:
  Adafruit_ST7735 &tft;
  InputHandler &inputHandler;
  
  const char* menuItems[4] = {"Space Invador", "Flappy Bird", "Snake", "Breakout"};
  int menuItemCount;
  
  void drawMenu();
};

#endif