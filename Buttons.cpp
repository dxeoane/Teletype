#include "config.h"

#include <Arduino.h>
#include "Buttons.h"
#include "Screen.h"

#ifdef BUTTONS_ENABLED    

    void setupButtons(){
        // Configuramos los pines de los botones como entradas con PULLUP
        pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP); 
        pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP); 
    }

    // Segun el hardware los botones se leen de forma diferente
    uint8_t doRead() {
        uint8_t buttons = 0;
        if (!digitalRead(BLUE_BUTTON_PIN)) {
            buttons |= BLUE_BUTTON;
        }
        if (!digitalRead(YELLOW_BUTTON_PIN)) {
            buttons |= YELLOW_BUTTON;
        }
        return buttons;
    }

#elif defined(TOUCHSCREEN_ENABLED)
    #include <SPI.h>
    #include <XPT2046_Touchscreen.h>

    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33

    SPIClass touchscreenSPI = SPIClass(VSPI);
    XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

    void setupButtons(){
        touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
        touchscreen.begin(touchscreenSPI);
        touchscreen.setRotation(1);
    }

    // Segun el hardware los botones se leen de forma diferente
    uint8_t doRead(void) {
        int x, y, z;

        uint8_t buttons = 0;
        if (touchscreen.tirqTouched() && touchscreen.touched()) {
            TS_Point p = touchscreen.getPoint();
            x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
            // y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
            z = p.z;
            if (z > 1000) {
                if (x < SCREEN_WIDTH / 2) {
                  buttons |= BLUE_BUTTON;  
                }
                if (x > SCREEN_WIDTH / 2) {
                  buttons |= YELLOW_BUTTON;  
                }
            }
        }

        return buttons;
    }
     
#else   
    void setupButtons(){

    }

    // Si no hay botons devolvemos siempre 0
    uint8_t doRead() {
        return 0;
    } 
    
#endif


// Lee el estado de los botones
uint8_t readButtons(void) {
    static unsigned long previousMillisButton = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisButton < 200) {
        return 0;
    }
    previousMillisButton = currentMillis;

    return doRead();
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
    
    uint8_t currentButtons = doRead();        
    uint8_t changedButtons = currentButtons & ~previousButtons; 
    previousButtons = currentButtons;

    return changedButtons;
}


