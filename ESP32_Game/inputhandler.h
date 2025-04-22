#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <Arduino.h>

class InputHandler {
public:
  InputHandler(int buttonPin, int xPin, int yPin);
  
  void update();
  void reset();
  
  // Input states
  bool buttonPressed = false;
  bool buttonReleased = false;
  bool buttonHeld = false;
  
  int xValue = 0;
  int yValue = 0;
  
  // Direction states (with deadzone)
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;
  
private:
  int _buttonPin;
  int _xPin;
  int _yPin;
  
  bool _lastButtonState = false;
  unsigned long _lastButtonPressTime = 0;
};

#endif