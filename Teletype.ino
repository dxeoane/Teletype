#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Screen.h"
#include "Buttons.h"
#include "Utils.h"

// Aqui van los parámetros que no se deben subir al repo de github (ssid, password, etc ...)
#include "secret.h"

#define PORT 8888
#define CHANNELS 3

unsigned long lastCheckMillis = 0;
char ACK[] = "ACK\r\n";  

WiFiUDP Udp;

void setup() {
  char spinner[] = "-\\|/";
  
  delay(1000);
  Serial.begin(115200);  

  setupButtons();  
  setupScreen(); 

  setTextSize(4);
  println("{ Teletype }");
  Serial.println("\n\n=== Teletype ===\n");
  
  setTextSize(2);  
  println("");

  printF("MAC: %s\n", WiFi.macAddress().c_str());
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str()); 
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);  
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 100) {
    printF("Conectando ..%c", spinner[retries % (sizeof(spinner)-1)]);
    setTextCursor(1, getTextCursorY());
    retries++;
    delay(100);
  }

  erase(0, getCursorY(), SCREEN_WIDTH, getCursorY() + (8*2));
  setTextCursor(1, getTextCursorY());

  if (WiFi.status() == WL_CONNECTED) {
    printF("Red: %s\n", STASSID);
    Serial.printf("Red: %s\n", STASSID);  
    printF("IP: %s\n", WiFi.localIP().toString().c_str());  
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());  
    printF("Gateway: %s\n", WiFi.gatewayIP().toString().c_str()); 
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());   
    printF("DNS: %s\n", WiFi.dnsIP().toString().c_str()); 
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());   
    int rssi = WiFi.RSSI();    
    printF("RSSI: %s\n", formatSignalStrength(rssi).c_str());  
    Serial.printf("RSSI: %d\n", rssi);   
    
    Udp.begin(PORT);    
    Serial.printf("Puerto: UDP/%d\n", PORT);   
    printF("Puerto: UDP/%d\n", PORT);  
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
      printF("AP SSID: %s\n", ssid.c_str());
      Serial.printf("AP SSID: %s\n", ssid.c_str());
      printF("IP: %s\n", WiFi.softAPIP().toString().c_str());  
      Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());   
            
      Udp.begin(PORT);      
      Serial.printf("Puerto: UDP/%d\n", PORT);   
      printF("Puerto: UDP/%d\n", PORT);   
    };        
  }

  Serial.printf("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);   
  printF("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);  
  
  setBWColors(); 
  setTextSize(2);  
}

void loop() {  
  
  enum Mode {
    TELETYPE,
    LOGGER
  };
  
  static Mode mode = TELETYPE;
  static int activeChannel = 0;

  uint8_t pressed = changedButtons();

  if (pressed == BLUE_BUTTON) {
    switch(mode) {
      case TELETYPE:
        mode = LOGGER;
        initLogger();
        Serial.println("Modo: Logger");      
        break;  
      case LOGGER:
        mode = TELETYPE;
        initTeletype();
        activeChannel = 0;
        Serial.println("Modo: Teletype");      
        break;        
    }
  }

  if (pressed == YELLOW_BUTTON) {
    switch(mode) {
      case TELETYPE:
        activeChannel = (activeChannel + 1) % CHANNELS;
        clear();
        printF("Channel %d\n", activeChannel);
        break;
      case LOGGER:
        initLogger();   
        break;
    }
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

  // Canal al que van los comandos, si no se especifica es 0
  int channel = 0;

  // Leemos los comandos linea a linea
  char *lasts;
  char *line = strtok_r(buffer, "\n\r", &lasts);  
  while (line) {   

    if (mode == LOGGER) {
      if (getCursorY() >= SCREEN_HEIGHT) {
        setTextCursor(1, 1);
      }
      setTextSize(1);
      setBWColors(); 
      // Borramos dos lineas
      erase(0, getCursorY(), SCREEN_WIDTH, 2*8); 
      println(line);
      hLine(0, getCursorY() + 4, SCREEN_WIDTH); 
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

    // Seleccionar el canal al que van dirigidos los comandos
    int i;
    if (sscanf(line, " channel %d", &i) == 1 && i >= 0 && i < CHANNELS) {
      channel = i;
    }

    // Si este comando va dirigido a un canal distinto del canal activo lo ignoramos
    if (activeChannel != channel) {
      line = strtok_r(NULL, "\n\r", &lasts);
      continue;
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
      else if (strcmp(command, "cursor") == 0) setTextCursor(params);      
      else if (strcmp(command, "size") == 0) setTextSize(params);
      // Comunes
      else if (strcmp(command, "bgcolor") == 0) setBgColor(params);
      else if (strcmp(command, "fgcolor") == 0) setFgColor(params);
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

  sendACK();
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
  } else {
    Serial.println("\nNo se pudo restablecer la conexion!");
  } 
 
  lastCheckMillis = now;
}

void initTeletype() {   
  setBWColors();   
  clear(); 
  setTextSize(4);
  println("{ Teletype }");  
  setTextSize(2);  
  println("");
  printF("MAC: %s\n", WiFi.macAddress().c_str());

  if (WiFi.status() == WL_CONNECTED) {
    printF("Red: %s\n", STASSID);
    printF("IP: %s\n", WiFi.localIP().toString().c_str());   
    printF("Gateway: %s\n", WiFi.gatewayIP().toString().c_str()); 
    printF("DNS: %s\n", WiFi.dnsIP().toString().c_str()); 
    int rssi = WiFi.RSSI(); 
    printF("RSSI: %s\n", formatSignalStrength(rssi).c_str());  
    printF("Puerto: UDP/%d\n", PORT);  
  } else {    
    printF("Red: Sin Conexion");      
  }
  printF("Display: %dX%d px\n", SCREEN_WIDTH, SCREEN_HEIGHT);  
}

void initLogger() {
  setBWColors();   
  setTextSize(1);
  clear();
  println("=== Logger ===");
}
