#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <Arduino.h>

#define BLUE_BUTTON       1
#define YELLOW_BUTTON     2

void setupButtons();
uint8_t readButtons(void);
uint8_t changedButtons(void);

#endif
