/****
 * Test sketch to set up NB-IOT connection on the Sodaq Mbili with ublox bee
 */

#include <Arduino.h>
#include "Sodaq_nbIOT.h"

// Mbili
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

const char* apn = "iot.orange.be";  // nb-iot network
//const char* forceOperator = "20610";

const char* udp = "52.166.32.29";  // AllThingsTalk endpoint
const char* port = "5684";

ATT_NBIOT nbiot;

/****
 *
 */
void setup()
{
  sodaq_wdt_safe_delay(STARTUP_DELAY);

  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  
  DEBUG_STREAM.println("Initializing and connecting... ");

  // Initialize the streams and turn on power to the bee module
  nbiot.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
  
  // Try connecting to the nb-iot network
  if(nbiot.connect(apn, udp, port))
    DEBUG_STREAM.println("Connected!");
  else
  {
    DEBUG_STREAM.println("Connection failed!");
    while(true) {}  // No connection. No need to continue the program
  }

  // Send one message
  nbiot.sendMessage("Hello from Ghent");
}

/****
 * Serial passthrough
 */
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
