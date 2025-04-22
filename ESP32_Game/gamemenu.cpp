#include "gamemenu.h"
#include "inputhandler.h"
#include <Arduino.h>

GameMenu::GameMenu(Adafruit_ST7735 &display, InputHandler &inputHandler) : 
  tft(display), inputHandler(inputHandler) {}

void GameMenu::init() {
    selectedItem = 0;
    currentGameIndex = -1;
    shouldLaunchGame = false; // Ensure launch state is reset
    menuItemCount = 4; // Space Invador, Flappy Bird, Snake, and Breakout
    menuItems[0] = "Space Invador";
    menuItems[1] = "Flappy Bird";
    menuItems[2] = "Snake";
    menuItems[3] = "Breakout";
    tft.fillScreen(MENU_BG);
    drawMenu();
}

void GameMenu::draw() {
    static int lastSelectedItem = -1;
    static bool lastButtonState = false;
    
    // Handle menu navigation with universal input controls and state tracking
    static unsigned long lastInputTime = 0;
    static bool wasUpPressed = false;
    static bool wasDownPressed = false;
    const unsigned long debounceDelay = 3500; // Reduced debounce time for better responsiveness
    
    unsigned long currentTime = millis();
    if (currentTime - lastInputTime >= debounceDelay) {
        // Handle UP button state transition
        if (inputHandler.up) {
            if (!wasUpPressed) { // Transition from not pressed to pressed
                selectedItem = max(0, selectedItem - 1);
                Serial.println("Menu: Selected item changed to " + String(selectedItem));
                lastInputTime = currentTime;
                wasUpPressed = true;
            }
        } else {
            wasUpPressed = false; // Reset state when button is released
        }
        
        // Handle DOWN button state transition
        if (inputHandler.down) {
            if (!wasDownPressed) { // Transition from not pressed to pressed
                selectedItem = min(menuItemCount - 1, selectedItem + 1);
                Serial.println("Menu: Selected item changed to " + String(selectedItem));
                lastInputTime = currentTime;
                wasDownPressed = true;
            }
        } else {
            wasDownPressed = false; // Reset state when button is released
        }
    }
    
    // Check if button is pressed to launch selected game
    if (inputHandler.buttonPressed) {
        Serial.println("Menu: Button pressed, attempting to launch game " + String(selectedItem));
        shouldLaunchGame = true;
        currentGameIndex = selectedItem;
        // Reset input states to prevent multiple triggers
        inputHandler.buttonPressed = false;
    }
    
    // Only redraw if selection changed or button was pressed
    if (lastSelectedItem != selectedItem || lastButtonState != inputHandler.buttonPressed) {
        drawMenu();
        lastSelectedItem = selectedItem;
        lastButtonState = inputHandler.buttonPressed;
    }
}

void GameMenu::drawMenu() {
    static int lastSelectedItem = -1;
    
    // Force full redraw when returning from game
    if (shouldLaunchGame) {
        lastSelectedItem = -1;
    }
    
    // Draw title (only once)
    if (lastSelectedItem == -1 || shouldLaunchGame) {
        tft.fillRect(0, 0, tft.width(), TITLE_HEIGHT, TITLE_COLOR);
        tft.setTextSize(TITLE_TEXT_SIZE);
        tft.setTextColor(WHITE);
        tft.setCursor((tft.width() - strlen("GAME MENU") * 12) / 2, 8);
        tft.print("GAME MENU");
        
        // Draw all menu items initially
        tft.setTextSize(MENU_TEXT_SIZE);
        tft.setTextColor(UNSELECTED_COLOR);
        for (int i = 0; i < menuItemCount; i++) {
            int itemY = MENU_START_Y + (i * MENU_ITEM_HEIGHT);
            int textX = (tft.width() - strlen(menuItems[i]) * 6) / 2;
            tft.setCursor(textX, itemY + MENU_ITEM_PADDING/2);
            tft.print(menuItems[i]);
        }
        
        // Reset launch state if coming from game over
        if (shouldLaunchGame) {
            shouldLaunchGame = false;
        }
    }
    
    tft.setTextSize(MENU_TEXT_SIZE);
    
    // Only redraw changed items
    if (lastSelectedItem != -1 && lastSelectedItem != selectedItem) {
        // Clear previous selection
        int prevY = MENU_START_Y + (lastSelectedItem * MENU_ITEM_HEIGHT);
        tft.fillRoundRect(2, prevY - 2, tft.width() - 4, MENU_ITEM_HEIGHT - 4, 4, MENU_BG);
        tft.setTextColor(UNSELECTED_COLOR);
        int textX = (tft.width() - strlen(menuItems[lastSelectedItem]) * 6) / 2;
        tft.setCursor(textX, prevY + MENU_ITEM_PADDING/2);
        tft.print(menuItems[lastSelectedItem]);
    }
    
    // Draw new selection
    int itemY = MENU_START_Y + (selectedItem * MENU_ITEM_HEIGHT);
    tft.fillRoundRect(2, itemY - 2, tft.width() - 4, MENU_ITEM_HEIGHT - 4, 4, SELECTOR_COLOR);
    tft.setTextColor(BLACK);
    int textX = (tft.width() - strlen(menuItems[selectedItem]) * 6) / 2;
    tft.setCursor(textX, itemY + MENU_ITEM_PADDING/2);
    tft.print(menuItems[selectedItem]);
    
    lastSelectedItem = selectedItem;
}