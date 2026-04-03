#ifndef _USER_DEFINED_CONFIG_H_
#define _USER_DEFINED_CONFIG_H_

// Habilita el modo de debug.
#define SERIAL_DEBUG_ENABLED

// Habilita la conexión MQTT
#define MQTT_ENABLED

// Habilita los botones
// #define BUTTONS_ENABLED
#define BLUE_BUTTON_PIN   D3
#define YELLOW_BUTTON_PIN D4

// Habilita la pantalla tactil
#define TOUCHSCREEN_ENABLED
#define TOUCHSCREEN_X_MIN 180
#define TOUCHSCREEN_X_MAX 3700
#define TOUCHSCREEN_Y_MIN 280
#define TOUCHSCREEN_Y_MAX 3800
#define TOUCHSCREEN_Z_THRESHOLD 1000
#define BLUE_BUTTON_RECT {1,220,20,20}
#define YELLOW_BUTTON_RECT {300,220,20,20}

#define BLUE_BUTTON_X1 0
#define BLUE_BUTTON_Y1 100

// Indica que la pantalla es especialmente pequeña
// #define TINY_SCREEN

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1

#endif