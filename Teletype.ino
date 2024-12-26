#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_ST7789.h>
#include <TimeLib.h>
#include "clock.h"
// Aqui van los parámetros que no se deben subir al repo de github (ssid, password, etc ...)
#include "secret.h"

#define BLUE_BUTTON       1
#define YELLOW_BUTTON     2
#define BLUE_BUTTON_PIN   D3
#define YELLOW_BUTTON_PIN D4

#define SCREEN_DC     D1 
#define SCREEN_RST    D2
#define SCREEN_CS     D8
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
// Usamos los caracteres desde el 33 hasta el 255, 223 posibles colores
#define PALETTE_SIZE  223 

Adafruit_ST7789 screen = Adafruit_ST7789(SCREEN_CS, SCREEN_DC, SCREEN_RST);

// Texto
uint16_t fgColor = ST77XX_WHITE;
uint16_t bgColor = ST77XX_BLACK;
uint8_t textSize = 2;
// Graficos
uint8_t scale  = 1;
uint16_t palette[PALETTE_SIZE];
uint16_t drawX = 0;
uint16_t drawY = 0;

#define PORT 8888
unsigned long lastCheckMillis = 0;
char ACK[] = "ACK\r\n";  

WiFiUDP Udp;

void setup() {
  char spinner[] = "-\\|/";
  
  delay(1000);
  Serial.begin(115200);  

  // Configuramos los pines de los botones como entradas con PULLUP
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP); 

  screen.init(SCREEN_HEIGHT, SCREEN_WIDTH, SPI_MODE0);  
  screen.setRotation(3);      
  screen.fillScreen(bgColor);
  screen.setTextColor(fgColor, bgColor);
  loadPalette(NULL);

  screen.setTextSize(4);
  screen.println("{ Teletype }");
  Serial.println("\n\n=== Teletype ===\n");
  
  screen.setTextSize(2);  
  screen.println("");

  screen.printf("MAC: %s\n", WiFi.macAddress().c_str());
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str()); 
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);  
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 100) {
    screen.printf("Conectando ..%c", spinner[retries % (sizeof(spinner)-1)]);
    screen.setCursor(0, screen.getCursorY());
    retries++;
    delay(100);
  }

  screen.print("                ");
  screen.setCursor(0, screen.getCursorY());

  if (WiFi.status() == WL_CONNECTED) {
    screen.printf("Red: %s\n", STASSID);
    Serial.printf("Red: %s\n", STASSID); 
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());   
    screen.printf("IP: %s\n", WiFi.localIP().toString().c_str());  
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());   
    screen.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str()); 
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());   
    screen.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str()); 
    int rssi = WiFi.RSSI();
    Serial.printf("RSSI: %d\n", rssi);   
    screen.printf("RSSI: %s\n", formatSignalStrength(rssi).c_str());  
    
    Udp.begin(PORT);    
    Serial.printf("Puerto: UDP/%d\n", PORT);   
    screen.printf("Puerto: UDP/%d\n", PORT);  
     // Inicializamos el reloj
     clockSetup();
  } else {    
    String macAddress = WiFi.softAPmacAddress();
    String lastTwoBytes = macAddress.substring(macAddress.length() - 5); 
    lastTwoBytes.replace(":", "");
    String ssid = "Teletype_" + lastTwoBytes; 

    IPAddress local(192,168,33,1);
    IPAddress gateway(192,168,33,1);
    IPAddress subnet(255,255,255,0);

    WiFi.softAPConfig(local, gateway, subnet);
    if (WiFi.softAP(ssid)) {      
      screen.printf("AP SSID: %s\n", ssid.c_str());
      Serial.printf("AP SSID: %s\n", ssid.c_str());
      Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());   
      screen.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());  
            
      Udp.begin(PORT);      
      Serial.printf("Puerto: UDP/%d\n", PORT);   
      screen.printf("Puerto: UDP/%d\n", PORT);   
    };        
  }

  Serial.printf("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);   
  screen.printf("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);  

  screen.print("Color: ");
  drawGradientBar();
  
  screen.setTextColor(fgColor, bgColor);
  screen.setTextSize(textSize);  
}

void loop() {  
  
  enum Mode {
    TELETYPE,
    DASHBOARD,
    LOGGER
  };
  
  static Mode mode = TELETYPE;

  uint8_t pressed = changedButtons();

  if (pressed == BLUE_BUTTON) {
    switch(mode) {
      case TELETYPE:
        mode = DASHBOARD;
        initDashboard();    
        Serial.println("Modo: Dashboard");  
        break;
      case DASHBOARD:
        mode = LOGGER;
        initLogger();
        Serial.println("Modo: Logger");      
        break;  
      case LOGGER:
      mode = TELETYPE;
        initTeletype();
        Serial.println("Modo: Teletype");      
        break;        
    }
  }

  if (pressed == YELLOW_BUTTON) {
    switch(mode) {
      case TELETYPE:
        clear();
        break;
      case DASHBOARD:
        clear();   
        break;
      case LOGGER:
        initLogger();   
        break;
    }
  }

  switch (mode) {
    case DASHBOARD:
      updateDashboard();
      break;      
  }  
  
  static char buffer[UDP_TX_PACKET_MAX_SIZE + 1];  

  // Comprobamos si la WiFi sigue conectada y funcionando
  if (WiFi.getMode() == WIFI_STA) {
    checkWiFi();
    if (WiFi.status() != WL_CONNECTED) return;
  }

  // Recibimos un paquete UDP
  int size = Udp.parsePacket();
  if (!size) return;

  // Si acabamos de recibir un paquete UDP damos por supuesto que la WiFi funciona bien
  lastCheckMillis = millis(); 
  
  buffer[Udp.read(buffer, UDP_TX_PACKET_MAX_SIZE)] = 0;

  // Leemos los comandos linea a linea
  char *lasts;
  char *line = strtok_r(buffer, "\n\r", &lasts);  
  while (line) {   

    if (mode == LOGGER) {
      if (screen.getCursorY() >= SCREEN_HEIGHT) {
        screen.setCursor(0, 0);
      }
      // Borramos dos lineas
      screen.fillRect(0, screen.getCursorY(), SCREEN_WIDTH, 2*8, ST77XX_BLACK); 
      screen.println(line);
      screen.drawFastHLine(0, screen.getCursorY() + 4, SCREEN_WIDTH, ST77XX_YELLOW); 
      line = strtok_r(NULL, "\n\r", &lasts);
      continue;
    }

    // Si es una linea con gran cantidad de datos no imprimimos todo, no tiene sentido
    if (strncmp(line, "scanline ", 9) == 0) {
      Serial.printf("Rcv (%s): scanline +%d bytes\n", Udp.remoteIP().toString().c_str(), strlen(line) - 9);  
    } else if (strncmp(line, "palette ", 8) == 0) {
      Serial.printf("Rcv (%s): palette +%d bytes\n", Udp.remoteIP().toString().c_str(), strlen(line) - 8);  
    } else {
      Serial.printf("Rcv (%s): %s\n", Udp.remoteIP().toString().c_str(), line);  
    }  

    char *command = strtok(line, " ");
    if (!command) {
      line = strtok_r(NULL, "\n\r", &lasts);
      continue;
    }     

    if (strcmp(command, "clear") == 0) clear(); 
    else {
      // El resto de comandos tienen parametros
      char *params = strtok(NULL, "\n\r");
      if (!params) {
        line = strtok_r(NULL, "\n\r", &lasts);
        continue;
      }

      // Texto
      if (strcmp(command, "print") == 0) print(params);
      else if (strcmp(command, "println") == 0) println(params);    
      else if (strcmp(command, "cursor") == 0) setCursor(params);      
      else if (strcmp(command, "size") == 0) setSize(params);
      // Comunes
      else if (strcmp(command, "bgcolor") == 0) setBackgroundColor(params);
      else if (strcmp(command, "fgcolor") == 0) setForegroundColor(params);
      // Graficos          
      else if (strcmp(command, "line") == 0) drawLine(params);        
      else if (strcmp(command, "hline") == 0) hLine(params);       
      else if (strcmp(command, "vline") == 0) vLine(params);      
      else if (strcmp(command, "rect") == 0) rect(params);      
      else if (strcmp(command, "fillrect") == 0) fillRect(params);       
      else if (strcmp(command, "erase") == 0) erase(params);     
      else if (strcmp(command, "circle") == 0) circle(params);       
      else if (strcmp(command, "fillcircle") == 0) fillCircle(params);  
      else if (strcmp(command, "roundrect") == 0) roundRect(params);       
      else if (strcmp(command, "fillroundrect") == 0) fillRoundRect(params); 
      else if (strcmp(command, "draw") == 0) setDrawPos(params);      
      else if (strcmp(command, "scale") == 0) setScale(params);       
      else if (strcmp(command, "palette") == 0) loadPalette(params);      
      else if (strcmp(command, "scanline") == 0) drawScanline(params); 
    }
    
    line = strtok_r(NULL, "\n\r", &lasts);
  }

  if (mode != LOGGER) sendACK();
}

void sendACK() {
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(ACK);
  Udp.endPacket();
}

WiFiClient client;

void checkWiFi() {

  unsigned long now = millis(); 
  if (now - lastCheckMillis < 5000) return;
  lastCheckMillis = now;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect("192.168.10.1", 80)) {
      client.stop();
      return;
    }
    client.stop();
  }
  
  Serial.println("No hay conexion WiFi!");
  Udp.stopAll();
  WiFi.disconnect();  

  Serial.print("Conectando ...");
  WiFi.begin(STASSID, STAPSK);  
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConexion WiFi restablecida.");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Udp.begin(PORT);  
    Serial.printf("Puerto: UDP/%d\n", PORT); 
    // Inicializamos el reloj de nuevo 
    clockSetup();
  } else {
    Serial.println("\nNo se pudo restablecer la conexion!");
  } 
 
  lastCheckMillis = now;
}

// Dibuja una barrita con el gradiente de colores
void drawGradientBar() {
  int x = screen.getCursorX(); 
  int y = screen.getCursorY(); 
  int width = 100;
  int height = 8*textSize;
  for (int i = 0; i < width / 2; i++) {
    uint8_t red = map(i, 0, width / 2 - 1, 255, 0); 
    uint8_t green = map(i, 0, width / 2 - 1, 0, 255);
    uint16_t color = screen.color565(red, green, 0);
    screen.drawFastVLine(x + i, y, height, color);
  }
  for (int i = width / 2; i < width; i++) {
    uint8_t green = map(i, width / 2, width - 1, 255, 0);
    uint8_t blue = map(i, width / 2, width - 1, 0, 255);
    uint16_t color = screen.color565(0, green, blue); 
    screen.drawFastVLine(x + i, y, height, color);
  }
}

// Formatea la calidad de señal del WiFi
String formatSignalStrength(int rssi) {
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;                // Sin señal
  } else if (rssi >= -50) {
    quality = 100;              // Señal máxima
  } else {
    quality = 2 * (rssi + 100); // Escala lineal entre -100 y -50 dBm
  }

  int bars = quality / 20;
  String signalText = "[";

  for (int i = 0; i < 5; i++) {
    if (i < bars) {
      signalText += "|"; 
    } else {
      signalText += " ";
    }
  }

  signalText += "] ";

  if (quality >= 80) {
    signalText += "Excelente";
  } else if (quality >= 60) {
    signalText += "Buena";
  } else if (quality >= 40) {
    signalText += "Regular";
  } else if (quality >= 20) {
    signalText += "Baja";
  } else {
    signalText += "";
  }

  return signalText;
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
void setSize(char *params) {
  int size; 
  if (sscanf(params, "%d", &size) == 1) {
    if (size<1 || size>100) return;
    textSize = size;    
    screen.setTextSize(size);
  }  
} 

// Convierte un color RGB, en hexadecimal, a RGB565
uint16_t strToColor(char* s) {
    unsigned int hexValue;
    if (sscanf(s, "%x", &hexValue) != 1) {
        return 0;
    }
    uint8_t r = (hexValue >> 16) & 0xFF;
    uint8_t g = (hexValue >> 8) & 0xFF; 
    uint8_t b = hexValue & 0xFF;
    return screen.color565(r, g, b);
}

void setBackgroundColor(char *params) {
  bgColor = strToColor(params); 
  screen.setTextColor(fgColor, bgColor);
} 

void setForegroundColor(char *params) {    
  fgColor = strToColor(params);  
  screen.setTextColor(fgColor, bgColor);
} 

void setCursor(char *params) {
  int x, y;
  if (sscanf(params, "%d,%d", &x, &y) == 2) {
    if (x<1 || x>100) return;
    if (y<1 || y>100) return;
    screen.setCursor((x-1)*6*textSize, (y-1)*8*textSize);
  }   
} 

void print(char *s) {
  screen.print(s);      
}

void println(char *s) {
  screen.println(s);      
}

// Fijamos la posicion donde dibujar
void setDrawPos(char *params) {
  int x, y;
  if (sscanf(params, "%d,%d", &x, &y) == 2) {
    drawX = x;
    drawY = y;  
  }   
} 

// Dibujamos una linea
void drawLine(char *params) {
  int x1, y1, x2, y2;
  if (sscanf(params, "%d,%d,%d,%d", &x1, &y1, &x2, &y2) == 4) {
    screen.drawLine(x1, y1, x2, y2, fgColor); 
  }   
} 

void hLine(char *params) {
  int x, y, w;
  if (sscanf(params, "%d,%d,%d", &x, &y, &w) == 3) {
    screen.drawFastHLine(x, y, w, fgColor); 
  }   
} 

void vLine(char *params) {
  int x, y, h;
  if (sscanf(params, "%d,%d,%d", &x, &y, &h) == 3) {
    screen.drawFastVLine(x, y, h, fgColor); 
  }   
} 

void rect(char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    screen.drawRect(x, y, w, h, fgColor); 
  }   
} 

void fillRect(char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    screen.fillRect(x, y, w, h, fgColor); 
  }   
} 

void erase(char *params) {
  int x, y, w, h;
  if (sscanf(params, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
    screen.fillRect(x, y, w, h, bgColor); 
  }   
} 

void circle(char *params) {
  int x, y, r;
  if (sscanf(params, "%d,%d,%d", &x, &y, &r) == 3) {
    screen.drawCircle(x, y, r, fgColor); 
  }   
} 

void fillCircle(char *params) {
  int x, y, r;
  if (sscanf(params, "%d,%d,%d", &x, &y, &r) == 3) {
    screen.fillCircle(x, y, r, fgColor); 
  }   
} 

void roundRect(char *params) {
  int x, y, w, h, r;
  if (sscanf(params, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &r) == 5) {
    screen.drawRoundRect(x, y, w, h, r, fgColor); 
  }   
} 

void fillRoundRect(char *params) {
  int x, y, w, h, r;
  if (sscanf(params, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &r) == 5) {
    screen.fillRoundRect(x, y, w, h, r, fgColor); 
  }   
} 

// Carga la paleta de colores, los colores deben estar en formato RGB565 (16 bits)
void loadPalette(char *s) {
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
void setScale(char *params) {
  int s; 
  if (sscanf(params, "%d", &s) == 1) {
    if (s<1 || s>100) return;
    scale = s;    
  }  
} 

void drawScanline(char *s) {  
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

String lpad(int number, int n) {
  String s = String(abs(number));
  if (number >= 0) {
    while (s.length() < n) s = "0" + s; 
  } else {
    while (s.length() < n - 1) s = "0" + s; 
    s = '-' + s;
  }
  return s;
}

void initTeletype() {      
  screen.fillScreen(bgColor);
  screen.setCursor(0, 0);
  screen.setTextColor(fgColor, bgColor);
  screen.setTextSize(4);
  screen.println("{ Teletype }");  
  screen.setTextSize(2);  
  screen.println("");
  screen.printf("MAC: %s\n", WiFi.macAddress().c_str());

  if (WiFi.status() == WL_CONNECTED) {
    screen.printf("Red: %s\n", STASSID);
    screen.printf("IP: %s\n", WiFi.localIP().toString().c_str());   
    screen.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str()); 
    screen.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str()); 
    int rssi = WiFi.RSSI(); 
    screen.printf("RSSI: %s\n", formatSignalStrength(rssi).c_str());  
    screen.printf("Puerto: UDP/%d\n", PORT);  
  } else {    
    screen.println("Red: Sin Conexion");      
  }
  screen.printf("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);  
  screen.print("Color: ");
  drawGradientBar();  
  screen.setTextColor(fgColor, bgColor);
  screen.setTextSize(textSize);
}

void initDashboard() {
  
}

void updateDashboard() {
  
  static unsigned long lastUpdate = 0;
  static const String weekDays[] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
  static const String months[] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};

  unsigned long now = millis(); 
  if (now - lastUpdate < 1000) {
    return;
  }
  lastUpdate = now;

  // Dibujamos el reloj
  textSize = 3;
  screen.setTextSize(textSize);
  screen.setCursor(0, SCREEN_HEIGHT - 8*textSize);
  String timenow = weekDays[weekday() - 1] + " " + lpad(day(),2) + " " + months[month()-1]
    + ". " + lpad(hour(),2) + ":" + lpad(minute(),2);
  screen.print(timenow);  
}

void initLogger() {
  screen.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  screen.setTextSize(1);
  screen.setTextWrap(false);
  screen.fillScreen(ST77XX_BLACK);
  screen.setCursor(0, 0);
  screen.println("=== Logger ===");
}
