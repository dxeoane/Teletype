#ifndef _SCREEN_H_
#define _SCREEN_H_

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_ORANGE 0xFC00

void setupScreen();

void clear();

void setTextSize(const char *params);
void setTextSize(uint8_t size);
void setBgColor(const char *params);
void setBgColor(uint16_t color);
void setFgColor(const char *params);
void setFgColor(uint16_t color);
void setBWColors();
void setTextCursor(const char *params);
void setTextCursor(uint16_t x, uint16_t y);
uint16_t getTextCursorX();
uint16_t getTextCursorY();
uint16_t getCursorX();
uint16_t getCursorY();
void print(const char *s);
void println(const char *s);
void printF(const char *format, ...);

void setDrawPos(const char *params);
void setDrawPos(uint16_t x, uint16_t y);
void drawLine(const char *params);
void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void hLine(const char *params);
void hLine(uint16_t x, uint16_t y, uint16_t w);
void vLine(const char *params);
void vLine(uint16_t x, uint16_t y, uint16_t h);
void rect(const char *params);
void rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void fillRect(const char *params);
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void erase(const char *params);
void erase(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void circle(const char *params);
void circle(uint16_t x, uint16_t y, uint16_t r);
void fillCircle(const char *params);
void fillCircle(uint16_t x, uint16_t y, uint16_t r);
void roundRect(const char *params);
void roundRect(uint16_t x, uint16_t y, uint16_t  w, uint16_t h, uint16_t r);
void fillRoundRect(const char *params);
void fillRoundRect(uint16_t yx, uint16_t yy, uint16_t yw, uint16_t yh, uint16_t yr);
void setScale(const char *params); 
void setScale(uint8_t s);
void loadPalette(const char *s);
void drawScanline(const char *s);

#endif
