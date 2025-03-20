#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_ST7789.h>
#include "Screen.h"

#define SCREEN_DC     D1 
#define SCREEN_RST    D2
#define SCREEN_CS     D8
// Usamos los caracteres desde el 33 hasta el 255, 223 posibles colores
#define PALETTE_SIZE  223 

// Texto
uint16_t fgColor = COLOR_WHITE;
uint16_t bgColor = COLOR_BLACK;
uint8_t textSize = 2;
// Graficos
uint8_t scale  = 1;
uint16_t palette[PALETTE_SIZE];
uint16_t drawX = 0;
uint16_t drawY = 0;

Adafruit_ST7789 screen = Adafruit_ST7789(SCREEN_CS, SCREEN_DC, SCREEN_RST);

void setupScreen() {
  screen.init(SCREEN_HEIGHT, SCREEN_WIDTH, SPI_MODE0);  
  screen.setSPISpeed(40000000);  
  screen.setRotation(3);      
  screen.fillScreen(bgColor);
  screen.setTextColor(fgColor, bgColor);
  screen.setTextWrap(false);
  loadPalette(NULL);
}

// Cubre toda la pantalla con el color de fondo
void clear() {
  screen.fillScreen(bgColor);    
  //  Ponemos el cursor es la esquina superior izquierda  
  screen.setCursor(0, 0);
  drawX = 0;
  drawY = 0;
} 

// Fija el tamaño con el que se imprime el texto x1, x2, x3, etc ...
void setTextSize(const char *params) {
  int size; 
  if (sscanf(params, "%d", &size) == 1) {
    setTextSize(size);
  }  
} 

void setTextSize(uint8_t size) {
  if (size<1 || size>16) return;
  textSize = size;  
  screen.setTextSize(size);
}

// Convierte un color RGB, en hexadecimal, a RGB565
uint16_t strToColor(const char* s) {
    unsigned int hexValue;
    if (sscanf(s, "%x", &hexValue) != 1) {
        return 0;
    }
    uint8_t r = (hexValue >> 16) & 0xFF;
    uint8_t g = (hexValue >> 8) & 0xFF; 
    uint8_t b = hexValue & 0xFF;
    return screen.color565(r, g, b);
}

void setBgColor(const char *params) {
  bgColor = strToColor(params); 
  screen.setTextColor(fgColor, bgColor);
} 

void setBgColor(uint16_t color)  {
  bgColor = color; 
  screen.setTextColor(fgColor, bgColor);
} 

void setFgColor(const char *params) {    
  fgColor = strToColor(params);  
  screen.setTextColor(fgColor, bgColor);
} 

void setFgColor(uint16_t color) {    
  fgColor = color;  
  screen.setTextColor(fgColor, bgColor);
} 

void setBWColors() {
  bgColor = COLOR_BLACK;
  fgColor = COLOR_WHITE;
  screen.setTextColor(fgColor, bgColor);
}

void setTextCursor(const char *params) {
  int x, y;
  if (sscanf(params, "%d,%d", &x, &y) == 2) {
    setTextCursor(x,y);
  }   
} 

void setTextCursor(uint16_t x, uint16_t y) {
  if (x<1 || x>256) return;
  if (y<1 || y>256) return;
  screen.setCursor((x-1)*6*textSize, (y-1)*8*textSize);
} 

uint16_t getTextCursorX() {
  return (screen.getCursorX() / (6*textSize)) + 1;
}

uint16_t getTextCursorY() {
  return (screen.getCursorY() / (8*textSize)) + 1;
}

uint16_t getCursorX() {
  return screen.getCursorX();
}

uint16_t getCursorY() {
  return screen.getCursorY();
}

void print(const char *s) {
  screen.print(s);      
}

void println(const char *s) {
  screen.println(s);      
}

void printF(const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  screen.print(buffer);
}

// Fijamos la posicion donde dibujar
void setDrawPos(const char *params) {
  int x, y;
  if (sscanf(params, "%d,%d", &x, &y) == 2) {
    setDrawPos(x, y); 
  }   
} 

void setDrawPos(uint16_t x, uint16_t y){
  drawX = x;
  drawY = y; 
} 

void drawLine(const char *params) {
  int x1, y1, x2, y2;
  if (sscanf(params, "%d,%d,%d,%d", &x1, &y1, &x2, &y2) == 4) {
    drawLine(x1, y1, x2, y2); 
  }   
} 

void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  screen.drawLine(x1, y1, x2, y2, fgColor);
} 

void hLine(const char *params) {
  int x, y, w;
  if (sscanf(params, "%d,%d,%d", &x, &y, &w) == 3) {
    hLine(x, y, w); 
  }   
} 

void hLine(uint16_t x, uint16_t y, uint16_t w) {
  screen.drawFastHLine(x, y, w, fgColor); 
} 


void vLine(const char *params) {
  int x, y, h;
  if (sscanf(params, "%d,%d,%d", &x, &y, &h) == 3) {
    vLine(x, y, h); 
  }   
} 

void vLine(uint16_t x, uint16_t y, uint16_t h) {
  screen.drawFastVLine(x, y, h, fgColor); 
} 

void rect(const char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    rect(x, y, w, h); 
  }   
} 

void rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  screen.drawRect(x, y, w, h, fgColor); 
} 

void fillRect(const char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    fillRect(x, y, w, h); 
  }   
} 

void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  screen.fillRect(x, y, w, h, fgColor); 
} 

void erase(const char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    erase(x, y, w, h); 
  }   
} 

void erase(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  screen.fillRect(x, y, w, h, bgColor); 
} 

void circle(const char *params) {
  int x, y, r;
  if (sscanf(params, "%d,%d,%d", &x, &y, &r) == 3) {
    circle(x, y, r); 
  }   
} 

void circle(uint16_t x, uint16_t y, uint16_t r) {
  screen.drawCircle(x, y, r, fgColor); 
} 

void fillCircle(const char *params) {
  int x, y, r;
  if (sscanf(params, "%d,%d,%d", &x, &y, &r) == 3) {
    fillCircle(x, y, r); 
  }   
} 

void fillCircle(uint16_t x, uint16_t y, uint16_t r) {
  screen.fillCircle(x, y, r, fgColor); 
} 

void roundRect(const char *params) {
  int x, y, w, h, r;
  if (sscanf(params, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &r) == 5) {
    roundRect(x, y, w, h, r); 
  }   
} 

void roundRect(uint16_t x, uint16_t y, uint16_t  w, uint16_t h, uint16_t r) {
  screen.drawRoundRect(x, y, w, h, r, fgColor); 
} 

void fillRoundRect(const char *params) {
  int x, y, w, h, r;
  if (sscanf(params, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &r) == 5) {
    fillRoundRect(x, y, w, h, r); 
  }   
} 

void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {
  screen.fillRoundRect(x, y, w, h, r, fgColor); 
} 

// Carga la paleta de colores, los colores deben estar en formato RGB565 (16 bits)
void loadPalette(const char *s) {
  // Inicializamos todos los colores de la paleta a negro
  memset(palette, 0xff, sizeof(palette));
  
  // Si no hay paleta que cragar salimos
  if (s == NULL) return; 

  // Cada color se representa con su valor hexadecimal, 4 letras cada uno
  int size = strlen(s) / 4;
  // Si la paleta supera el tamaño maximo, solo cargamos hasta el tamaño maximo
  if (size > PALETTE_SIZE) {
    size = PALETTE_SIZE;
  }  
  // Leemos los colores uno a uno
  for (int i = 0; i < size; i++) {
      sscanf(&s[i * 4], "%4hx", &palette[i]);
  }
}

// El valor "scale" indica cuanto se debe ampliar una imagen al dibujarla (x1, x2, 3x, etc ...)
void setScale(const char *params) {
  int s; 
  if (sscanf(params, "%d", &s) == 1) {
    setScale(s);  
  }  
} 

void setScale(uint8_t s) {
  if (s<1 || s>100) return;
  scale = s; 
} 

void drawScanline(const char *s) {  
  if (s == NULL) return; 
  
  int len = strlen(s);

  // Comenzamos la escritura en la pantalla
  screen.startWrite();    

  // Repetimos la misma linea horizontal tantas veces como queramos escalar la imagen (x1, x2, etc ...)
  for (int j = 0; j < scale; j++) {

    // Si la linea está por encima del margen superior de la pantalla, pasamos a la siguiente linea
    if (drawY < 0) {
      drawY++;
      continue;
    }
    // Si la linea está por debajo del margen inferior, hemos terminado
    if (drawY >= SCREEN_HEIGHT) {
      drawY++;
      break;
    }

    // Volvemos al principio de la linea
    int16_t x = drawX;
    
    for (int i = 0; i < len; i++) {  
      char c = s[i];
      // Usamos los caracteres desde el 33 hasta el 255, 223 posibles colores.
      if (c < 33) {
        // Si no es un color, lo ignoramos
        x += scale;
      } else {
        for (int k = 0; k < scale; k ++){
          // El pixel está a la izquierda del margen de la pantalla, pasamos al siguiente pixel
          if (x < 0) {
            x++;
            continue;
          }
          // El pixel está a la derecha del margen de la pantalla, hemos terminado con esta linea
          if (x >= SCREEN_WIDTH){
            break;
          }
          // Escribimos el pixel, usando el color de la paleta. 
          screen.writePixel(x,drawY,palette[c-33]);
          x++;
        }        
      }
    }
    // Pasamos a la siguiente linea
    drawY++;
  }  

  // Terminamos la escritura
  screen.endWrite();
}
