#include <Arduino.h>
#include "Sodaq_nbIOT.h"
#include <Sodaq_wdt.h>

// Mbili support
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

#define STARTUP_DELAY 3000

// nb-iot network
const char* apn = "iot.orange.be";
//const char* forceOperator = "20610";

// AllThingsTalk endpoint
const char* udp = "52.166.32.29";
const char* port = "5684";

Sodaq_nbIOT nbiot;

void setup()
{
  sodaq_wdt_safe_delay(STARTUP_DELAY);

  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  
  DEBUG_STREAM.println("Initializing and connecting... ");

  nbiot.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
  
  if(nbiot.connect(apn, udp, port))
    DEBUG_STREAM.println("Connected!");
  else
  {
    DEBUG_STREAM.println("Connection failed!");
    while(true) {}  // No connection. No need to continue the program
  }
}

int counter = 1;  // Initialize counter
unsigned long sendNextAt = 0;  // Keep track of time
void loop() 
{
  if(sendNextAt < millis())
  {
    nbiot.sendMessage(String(counter));
    counter++;
    sendNextAt = millis() + 10000;
  }
}
