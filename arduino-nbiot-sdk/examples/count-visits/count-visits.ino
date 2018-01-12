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

#include <PayloadBuilder.h>
#include "ATT_NBIOT.h"

// Mbili support
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

// AllThingsTalk Device
const char* deviceid = "Ddjdc1aYskfXGva9z6gelQWO";
const char* devicetoken = "spicy:4O3SMDqz3ygu80Nw7ybfJYdrR1FwCzN5fMFwuTD1";

ATT_NBIOT nbiot;
PayloadBuilder payload(nbiot);

int pushButton = 20;          
int doorSensor = 4;

#define SEND_MAX_EVERY 30000 // the (mimimum) time between 2 consecutive updates of visit counts

bool prevButtonState;
bool prevDoorSensor;

short visitCount = 0;  // keep track of the nr of visitors
short prevVisitCountSent = 0;
unsigned long lastSentAt = 0;  // the time when the last visitcount was sent to the cloud

void setup() 
{
  pinMode(pushButton, INPUT);  // initialize the digital pin as an input
  pinMode(doorSensor, INPUT);

  delay(3000);

  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  
  DEBUG_STREAM.println("Initializing and connecting... ");

  nbiot.setAttDevice(deviceid, devicetoken);
  nbiot.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
  
  if(nbiot.connect())
    DEBUG_STREAM.println("Connected!");
  else
  {
    DEBUG_STREAM.println("Connection failed!");
    while(true) {}  // No connection. No need to continue the program
  }

  DEBUG_STREAM.println();
  DEBUG_STREAM.println("-- Count visits NB-IoT experiment --");
  DEBUG_STREAM.print("Sending data every ");
  DEBUG_STREAM.print(SEND_MAX_EVERY);
  DEBUG_STREAM.println(" milliseconds, if changed");
  DEBUG_STREAM.println();
  
  prevButtonState = digitalRead(pushButton);  // read the initial state
  prevDoorSensor = digitalRead(doorSensor);   // read the initial state
}

void sendVisitCount(int16_t val)
{
  payload.reset();
  payload.addInteger(val);
  payload.send(false);
  
  lastSentAt = millis();
  prevVisitCountSent = val;
}

void loop() 
{
  processButton();
  processDoorSensor();
  delay(100);
  
  // only send a message when something has changed and SEND_MAX_EVERY has been exceeded
  if(prevVisitCountSent != visitCount && lastSentAt + SEND_MAX_EVERY <= millis())
    sendVisitCount(visitCount);
}

// check the state of the door sensor
void processDoorSensor()
{
  bool sensorRead = digitalRead(doorSensor);                         
  if(prevDoorSensor != sensorRead)
  {
    prevDoorSensor = sensorRead;
    if(sensorRead == true)
    {
      DEBUG_STREAM.println("Door closed");
      visitCount++;  // the door was opened and closed again, so increment the counter
		  DEBUG_STREAM.print("VisitCount: ");
		  DEBUG_STREAM.println(visitCount);
    }
    else
      DEBUG_STREAM.println("Door open");
  }
}

// check the state of the button
void processButton()
{
  bool sensorRead = digitalRead(pushButton);  // check the state of the button
  if (prevButtonState != sensorRead)          // verify if value has changed
  {
    prevButtonState = sensorRead;
    if(sensorRead == true)                                         
    {
      DEBUG_STREAM.println("Button pressed, counter reset");
      visitCount = 0;
    }
    else
     DEBUG_STREAM.println("Button released");
  }
}