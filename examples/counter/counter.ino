/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2018 AllThingsTalk
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
/****
 * Basic example sending an integer counter to AllThingsTalk.
 * You can send the data either using standard Json, Cbor or a binary
 * payload.
 */
 
// Select your preferred method of sending data
//#define JSON
#define CBOR
//#define BINARY

#include "ATT_NBIOT.h"

// Mbili support
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

ATT_NBIOT device;

#ifdef CBOR
  #include <CborBuilder.h>
  CborBuilder payload(device);
#endif

#ifdef BINARY
  #include <PayloadBuilder.h>
  PayloadBuilder payload(device);
#endif


void setup()
{
  delay(3000);

  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  
  DEBUG_STREAM.println("Initializing and connecting... ");

  device.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
  
  if(device.connect())
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
    #ifdef JSON  // Send data using regular json
    device.sendMessage(counter, "counter");
    #endif

    #ifdef CBOR  // Send data using Cbor
    payload.reset();
    payload.map(1);
    payload.addInteger(counter, "counter");
    payload.send();
    #endif

    #ifdef BINARY  // Send data using binary payload, make sure the decoding is set at AllThingsTalk
    payload.reset();
    payload.addInteger(counter);
    payload.send();
    #endif

    counter++;
    sendNextAt = millis() + 10000;
  }
}