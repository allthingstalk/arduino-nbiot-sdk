#include <Arduino.h>
#include "Sodaq_nbIOT.h"
#include <Sodaq_wdt.h>

// Mbili
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

#define STARTUP_DELAY 3000

const char* apn = "iot.orange.be";  // nb-iot network
//const char* forceOperator = "20610";
const char* udp = "52.166.32.29";  // AllThingsTalk endpoint
const char* port = "5684";

Sodaq_nbIOT nbiot;

void sendMessage(const char* message);

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

  const char* message = "Hello Ghent";
  nbiot.sendMessage(message);
}

void loop()
{
  while (DEBUG_STREAM.available())
  {
    MODEM_STREAM.write(DEBUG_STREAM.read());
  }

  while (MODEM_STREAM.available())
  {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }
}
