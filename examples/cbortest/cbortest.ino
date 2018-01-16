#include "CborBuilder.h"
#include "ATT_NBIOT.h"

// Mbili support
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

ATT_NBIOT nbiot;

//Create object and Writer
CborBuilder cbor(nbiot);

void setup()
{
  delay(3000);

  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  DEBUG_STREAM.println("Initializing and connecting... ");
  nbiot.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
}

int counter = 101;  // Initialize counter
unsigned long sendNextAt = 0;  // Keep track of time
void loop() 
{
  if(sendNextAt < millis())
  {
    cbor.reset();
    cbor.map(1);  // Amount of datapoints to be sent
    //cbor.addInteger(3, "a");
    //cbor.addInteger(4, "b");
    //cbor.addGps(3, 4, 5, "gps");
    cbor.addBoolean(true, "c");
    cbor.printCbor();
   
    counter++;
    sendNextAt = millis() + 1000;
  }
}