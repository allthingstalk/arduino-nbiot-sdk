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

#include <Wire.h>
#include <SoftwareSerial.h>
#include <MMA7660.h>

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

#define MOVEMENTTRESHOLD 12  // threshold for accelerometer movement

MMA7660 accelemeter;

SoftwareSerial SoftSerial(20, 21);  // reading GPS values from debugSerial connection with GPS

unsigned char buffer[64];  // buffer array for data receive over debugSerial port
int count=0;  

// variables for the coordinates (GPS)
float latitude;
float longitude;
float altitude;
float timestamp;

int8_t prevX,prevY,prevZ;  // keeps track of the previous accelerometer data

// accelerometer data is translated to 'moving' vs 'not moving' on the device
// this boolean value is sent to the cloud using a generic 'Binary Sensor' container
bool wasMoving = false;     

void setup() 
{
  // accelerometer is always running so we can check when the object is moving
  accelemeter.init();
  SoftSerial.begin(9600); 

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
  DEBUG_STREAM.println("-- Guard your stuff LoRa experiment --");
  DEBUG_STREAM.println();
    
  DEBUG_STREAM.print("Initializing GPS. Get first fix");
  while(readCoordinates() == false)
  {
    delay(1000);
    DEBUG_STREAM.print(".");
  }
  DEBUG_STREAM.println("Done");

  accelemeter.getXYZ(&prevX, &prevY, &prevZ);  // get initial accelerometer state

  DEBUG_STREAM.println("Sending initial state");

  // send initial value
  payload.reset();
  payload.addBoolean(false);
  payload.send(false) ;

  DEBUG_STREAM.println("Ready to guard your stuff");
  DEBUG_STREAM.println();
}

void loop() 
{
  if(isMoving() == true)
  {   
    if(wasMoving == false)
    {
      DEBUG_STREAM.println();
      DEBUG_STREAM.println("Movement detected");
      DEBUG_STREAM.println("-----------------");

      wasMoving = true;
      sendCoordinates(true);
    }
  }
  else if(wasMoving == true)
  {
    DEBUG_STREAM.println();
    DEBUG_STREAM.println("Movement stopped");
    DEBUG_STREAM.println("----------------");
    
    // no need to send coordinates when the device has stopped moving
    // since the coordinates stay the same. This saves some power
    wasMoving = false;
    sendCoordinates(false);  // send over last known coordinates
  }
  delay(500);  // sample the accelerometer quickly
}

bool isMoving()
{
  int8_t x,y,z;
  accelemeter.getXYZ(&x, &y, &z);
  bool result = (abs(prevX - x) + abs(prevY - y) + abs(prevZ - z)) > MOVEMENTTRESHOLD;

  // do a second measurement to make sure movement really stopped
  if(result == false && wasMoving == true)
  {
    delay(800);
    accelemeter.getXYZ(&x, &y, &z);
    result = (abs(prevX - x) + abs(prevY - y) + abs(prevZ - z)) > MOVEMENTTRESHOLD;
  }
  
  if(result == true)
  {
    prevX = x;
    prevY = y;
    prevZ = z;
  }

  return result; 
}

// try to read the gps coordinates from the text stream that was received from the gps module
// returns true when gps coordinates were found in the input, otherwise false
bool readCoordinates()
{
  // sensor can return multiple types of data
  // we need to capture lines that start with $GPGGA
  bool foundGPGGA = false;
  if (SoftSerial.available())                     
  {
    while(SoftSerial.available())  // read data into char array
    {
      buffer[count++]=SoftSerial.read();  // store data in a buffer for further processing
      if(count == 64)
        break;
    }
    foundGPGGA = count > 60 && extractValues();  // if we have less then 60 characters, we have incomplete input
    clearBufferArray();
  }
  return foundGPGGA;
}

// send the GPS coordinates to the AllThingsTalk cloud
void sendCoordinates(boolean val)
{
  DEBUG_STREAM.print("Retrieving GPS coordinates for transmission, please wait");
  
  /*
   * try to read some coordinates until we have a valid set. Every time we
   * fail, pause a little to give the GPS some time. There is no point to
   * continue without reading gps coordinates. The bike was potentially
   * stolen, so the lcoation needs to be reported before switching back to
   * 'not moving'.
   */  
  
  while(readCoordinates() == false)
  {
    DEBUG_STREAM.print(".");
    delay(1000);                 
  }
  DEBUG_STREAM.println();
  
  payload.reset();
  payload.addBoolean(val);
  payload.addGPS(latitude, longitude, altitude);
  
  payload.send(false) ;
  
  DEBUG_STREAM.print("lng: ");
  DEBUG_STREAM.print(longitude, 4);
  DEBUG_STREAM.print(", lat: ");
  DEBUG_STREAM.print(latitude, 4);
  DEBUG_STREAM.print(", alt: ");
  DEBUG_STREAM.print(altitude);
  DEBUG_STREAM.print(", time: ");
  DEBUG_STREAM.println(timestamp);
}

// extract all the coordinates from the stream
// store the values in the globals defined at the top of the sketch
bool extractValues()
{
  unsigned char start = count;
  
  // find the start of the GPS data
  // if multiple $GPGGA appear in 1 line, take the last one
  while(buffer[start] != '$')
  {
    if(start == 0)  // it's unsigned char, so we can't check on <= 0
      break;
    start--;
  }
  start++;  // skip the '$', don't need to compare with that

  // we found the correct line, so extract the values
  if(start + 4 < 64 && buffer[start] == 'G' && buffer[start+1] == 'P' && buffer[start+2] == 'G' && buffer[start+3] == 'G' && buffer[start+4] == 'A')
  {
    start += 6;
    timestamp = extractValue(start);
    latitude = convertDegrees(extractValue(start) / 100);
    start = skip(start);    
    longitude = convertDegrees(extractValue(start)  / 100);
    start = skip(start);
    start = skip(start);
    start = skip(start);
    start = skip(start);
    altitude = extractValue(start);
    return true;
  }
  else
    return false;
}

float convertDegrees(float input)
{
  float fractional = input - (int)input;
  return (int)input + (fractional / 60.0) * 100.0;
}

// extracts a single value out of the stream received from the device and returns this value
float extractValue(unsigned char& start)
{
  unsigned char end = start + 1;
  
  // find the start of the GPS data
  // if multiple $GPGGA appear in 1 line, take the last one
  while(end < count && buffer[end] != ',')
    end++;

  // end the string so we can create a string object from the sub string
  // easy to convert to float
  buffer[end] = 0;
  float result = 0.0;

  if(end != start + 1)  // if we only found a ',' then there is no value
    result = String((const char*)(buffer + start)).toFloat();

  start = end + 1;
  return result;
}

// skip a position in the text stream that was received from the gps
unsigned char skip(unsigned char start)
{
  unsigned char end = start + 1;
  
  // find the start of the GPS data
  // if multiple $GPGGA appear in 1 line, take the last one
  while(end < count && buffer[end] != ',')
    end++;

  return end+1;
}

// reset the entire buffer back to 0
void clearBufferArray()
{
  for (int i=0; i<count;i++)
  {
    buffer[i]=NULL;
  }
  count = 0;
}