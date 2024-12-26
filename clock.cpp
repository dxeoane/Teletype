#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "Arduino.h"
#include "clock.h"

WiFiUDP UdpClock;
IPAddress ntpServerIPAddress; 

const int NTP_PACKET_SIZE = 48; 
byte ntpPacket[NTP_PACKET_SIZE]; 

void sendNTPpacket(){
  memset(ntpPacket, 0, NTP_PACKET_SIZE);
  ntpPacket[0] = 0b11100011;   // LI, Version, Mode
  ntpPacket[1] = 0;            // Stratum, or type of clock
  ntpPacket[2] = 6;            // Polling Interval
  ntpPacket[3] = 0xEC;         // Peer Clock Precision
  ntpPacket[12] = 49;
  ntpPacket[13] = 0x4E;
  ntpPacket[14] = 49;
  ntpPacket[15] = 52;
  
  UdpClock.beginPacket(ntpServerIPAddress, 123); // Port 123
  UdpClock.write(ntpPacket, NTP_PACKET_SIZE);
  UdpClock.endPacket();
}

time_t getTime(){ 

  // Discard all previous packets
  while (UdpClock.parsePacket() > 0); 
  
  Serial.println("Send NTP Request"); 
  sendNTPpacket();
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = UdpClock.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      UdpClock.read(ntpPacket, NTP_PACKET_SIZE); 
      unsigned long secsSince1900;
      // Convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)ntpPacket[40] << 24;
      secsSince1900 |= (unsigned long)ntpPacket[41] << 16;
      secsSince1900 |= (unsigned long)ntpPacket[42] << 8;
      secsSince1900 |= (unsigned long)ntpPacket[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response!");
  return 0;
}

void clockSetup() {
  UdpClock.begin(ntpLocalPort);
  WiFi.hostByName(ntpServer, ntpServerIPAddress);
  setSyncProvider(getTime);
  setSyncInterval(300);
}
