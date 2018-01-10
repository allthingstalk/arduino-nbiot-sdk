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
#include <ATT_GPS.h>

#include <Wire.h>
#include <MMA7660.h>

// Mbili support
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23

#define baud 9600

#define ACCEL_THRESHOLD 12  // Threshold for accelerometer movement
#define DISTANCE 30.0       // Minimal distance between two fixes to keep checking gps
#define FIX_DELAY 60000     // Delay (ms) between checking gps coordinates

// AllThingsTalk Device
const char* deviceid = "Ddjdc1aYskfXGva9z6gelQWO";
const char* devicetoken = "spicy:4O3SMDqz3ygu80Nw7ybfJYdrR1FwCzN5fMFwuTD1";

ATT_NBIOT nbiot;
PayloadBuilder payload(nbiot);
ATT_GPS gps(20,21);  // Reading GPS values from debugSerial connection with GPS

MMA7660 accelerometer;
bool moving = false;  // Set given data from both accelerometer and gps

// variables for the coordinates (GPS)
float prevLatitude;
float prevLongitude;

int8_t prevX,prevY,prevZ;  // Keeps track of the previous accelerometer data

void setup() 
{
  accelerometer.init();  // Accelerometer is always running so we can check if movement started

  delay(100);

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
  DEBUG_STREAM.println("-- Guard your stuff NB-IoT experiment --");
  DEBUG_STREAM.println();

  readCoordinates();  // Get initial gps fix
  accelerometer.getXYZ(&prevX, &prevY, &prevZ);  // Get initial accelerometer state

  DEBUG_STREAM.println("Ready to guard your stuff");
}

unsigned long sendNextAt = 0;  // Keep track of time
void loop()
{
  if(!moving)  // If not moving, check accelerometer
  {
    moving = isAccelerating();
    delay(500);
  }

  if(moving && sendNextAt < millis())  // We waited long enough to check new fix
  {
    readCoordinates();
    
    if(gps.calcDistance(prevLatitude, prevLongitude) <= DISTANCE)  // We did not move much. Back to checking accelerometer for movement
    {
      DEBUG_STREAM.print("Less than ");
      DEBUG_STREAM.print(DISTANCE);
      DEBUG_STREAM.println(" movement in last 5 minutes");
      moving = false;
      sendCoordinates(false);  // Send fix and motion false
    }
    else  // Update and send new coordinates
    {
      prevLatitude = gps.latitude;
      prevLongitude = gps.longitude;
      sendCoordinates(true);  // Send fix and motion true
    }
    sendNextAt = millis() + FIX_DELAY;  // Update time
  }
}

// Check if acceleration is detected
bool isAccelerating()
{
  int8_t x,y,z;
  accelerometer.getXYZ(&x, &y, &z);
  bool result = (abs(prevX - x) + abs(prevY - y) + abs(prevZ - z)) > ACCEL_THRESHOLD;
  
  if(result == true)
  {
    prevX = x;
    prevY = y;
    prevZ = z;
  }

  return result; 
}

// Try reading GPS coordinates
void readCoordinates()
{
  while(gps.readCoordinates() == false)
  {
    DEBUG_STREAM.print(".");
    delay(1000);                 
  }
  DEBUG_STREAM.println();
}

// send the GPS coordinates to the AllThingsTalk cloud
void sendCoordinates(boolean val)
{
  payload.reset();
  payload.addBoolean(val);
  payload.addGPS(gps.latitude, gps.longitude, gps.altitude);
  payload.send(false);
  
  DEBUG_STREAM.print("lng: ");
  DEBUG_STREAM.print(gps.longitude, 4);
  DEBUG_STREAM.print(", lat: ");
  DEBUG_STREAM.print(gps.latitude, 4);
  DEBUG_STREAM.print(", alt: ");
  DEBUG_STREAM.print(gps.altitude);
  DEBUG_STREAM.print(", time: ");
  DEBUG_STREAM.println(gps.timestamp);
}