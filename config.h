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

// Indica que la pantalla es especialmente pequeña
// #define TINY_SCREEN

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1

#endif