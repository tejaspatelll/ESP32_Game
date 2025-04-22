#include "inputhandler.h"

InputHandler::InputHandler(int buttonPin, int xPin, int yPin) : 
  _buttonPin(buttonPin), _xPin(xPin), _yPin(yPin) {
  pinMode(_buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("InputHandler initialized with pins:");
  Serial.print("Button: "); Serial.println(_buttonPin);
  Serial.print("X Axis: "); Serial.println(_xPin);
  Serial.print("Y Axis: "); Serial.println(_yPin);
}

void InputHandler::reset() {
  // Reset all input states
  buttonPressed = false;
  buttonReleased = false;
  buttonHeld = false;
  left = false;
  right = false;
  up = false;
  down = false;
  _lastButtonState = false;
  _lastButtonPressTime = 0;
}

void InputHandler::update() {
  // Reset direction states
  left = false;
  right = false;
  up = false;
  down = false;
  
  // Read analog inputs
  xValue = analogRead(_xPin);
  yValue = analogRead(_yPin);
  
  // Debug output
  Serial.print("Joystick - X: "); Serial.print(xValue);
  Serial.print(" Y: "); Serial.print(yValue);
  Serial.print(" | Button: "); Serial.print(digitalRead(_buttonPin));
  Serial.print(" | States: ");
  Serial.print(left ? "LEFT " : "");
  Serial.print(right ? "RIGHT " : "");
  Serial.print(up ? "UP " : "");
  Serial.print(down ? "DOWN " : "");
  Serial.println(buttonPressed ? "PRESSED" : "");
  
  // Process button state
  bool currentButtonState = digitalRead(_buttonPin) == HIGH;
  
  buttonPressed = false;
  buttonReleased = false;
  
  if (currentButtonState != _lastButtonState) {
    if (currentButtonState) {
      buttonPressed = true;
      _lastButtonPressTime = millis();
    } else {
      buttonReleased = true;
    }
    _lastButtonState = currentButtonState;
  }
  
  buttonHeld = currentButtonState && (millis() - _lastButtonPressTime > 200);
  
  // Process joystick directions with deadzone
  left = xValue < 1600;
  right = xValue > 1700;
  down = yValue < 1600;
  up = yValue > 1700;
}