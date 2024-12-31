#include <Arduino.h>
#include "Buttons.h"

#define BLUE_BUTTON_PIN   D3
#define YELLOW_BUTTON_PIN D4

void setupButtons(){
  // Configuramos los pines de los botones como entradas con PULLUP
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP); 
}

// Lee el estado de los botones
uint8_t readButtons(void) {
    static unsigned long previousMillisButton = 0;
    unsigned long currentMillis = millis();
  
    if (currentMillis - previousMillisButton < 200) {
        return 0;
    }
    previousMillisButton = currentMillis;

    uint8_t buttons = 0;
    if (!digitalRead(BLUE_BUTTON_PIN)) {
        buttons |= BLUE_BUTTON;
    }
    if (!digitalRead(YELLOW_BUTTON_PIN)) {
        buttons |= YELLOW_BUTTON;
    }

    return buttons;
}


// Devuelve los botones que han cambiado su estado a presionado
uint8_t changedButtons(void) {
    static uint8_t previousButtons = 0; 
    static unsigned long previousMillisButton = 0;
    unsigned long currentMillis = millis();    
  
    if (currentMillis - previousMillisButton < 200) {
        return 0; 
    }
    previousMillisButton = currentMillis; 
    
    uint8_t currentButtons = 0;
    if (!digitalRead(BLUE_BUTTON_PIN)) {
        currentButtons |= BLUE_BUTTON;
    }
    if (!digitalRead(YELLOW_BUTTON_PIN)) {
        currentButtons |= YELLOW_BUTTON;
    }
    
    uint8_t changedButtons = currentButtons & ~previousButtons; 
    previousButtons = currentButtons;

    return changedButtons;
}
