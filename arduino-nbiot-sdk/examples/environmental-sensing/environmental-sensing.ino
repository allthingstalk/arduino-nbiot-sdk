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
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "AirQuality2.h"

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

#define AirQualityPin A0
#define LightSensorPin A2
#define SoundSensorPin A4

#define SEND_EVERY 30000

AirQuality2 airqualitysensor;
Adafruit_BME280 tph; // I2C

float soundValue;
float lightValue;
float temp;
float hum;
float pres;
int16_t airValue;

void setup() 
{
  pinMode(GROVEPWR, OUTPUT);  // turn on the power for the secondary row of grove connectors
  digitalWrite(GROVEPWR, HIGH);

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
  DEBUG_STREAM.println("-- Environmental Sensing NB-IoT experiment --");
  DEBUG_STREAM.println();

  initSensors();
}

void loop() 
{
  readSensors();
  displaySensorValues();
  sendSensorValues();
  
  DEBUG_STREAM.print("Delay for: ");
  DEBUG_STREAM.println(SEND_EVERY);
  DEBUG_STREAM.println();
  delay(SEND_EVERY);
}

void initSensors()
{
  DEBUG_STREAM.println("Initializing sensors, this can take a few seconds...");
  
  pinMode(SoundSensorPin, INPUT);
  pinMode(LightSensorPin, INPUT);
  
  tph.begin();
  airqualitysensor.init(AirQualityPin);
  DEBUG_STREAM.println("Done");
}

void readSensors()
{
    DEBUG_STREAM.println("Start reading sensors");
    DEBUG_STREAM.println("---------------------");
    
    soundValue = analogRead(SoundSensorPin);
    lightValue = analogRead(LightSensorPin);
    lightValue = lightValue * 3.3 / 1023;  // convert to lux based on the voltage that the sensor receives
    lightValue = pow(10, lightValue);
    
    temp = tph.readTemperature();
    hum = tph.readHumidity();
    pres = tph.readPressure()/100.0;
    
    airValue = airqualitysensor.getRawData();
}

void sendSensorValues()
{
  payload.reset();
  payload.addNumber(soundValue);
  payload.addNumber(lightValue);
  payload.addNumber(temp);
  payload.addNumber(hum);
  payload.addNumber(pres);
  payload.addInteger(airValue);

  payload.send(false);
}

void displaySensorValues()
{
  DEBUG_STREAM.print("Sound level: ");
  DEBUG_STREAM.print(soundValue);
  DEBUG_STREAM.println(" Analog (0-1023)");
      
  DEBUG_STREAM.print("Light intensity: ");
  DEBUG_STREAM.print(lightValue);
  DEBUG_STREAM.println(" Lux");
      
  DEBUG_STREAM.print("Temperature: ");
  DEBUG_STREAM.print(temp);
  DEBUG_STREAM.println(" °C");
      
  DEBUG_STREAM.print("Humidity: ");
  DEBUG_STREAM.print(hum);
	DEBUG_STREAM.println(" %");
      
  DEBUG_STREAM.print("Pressure: ");
  DEBUG_STREAM.print(pres);
	DEBUG_STREAM.println(" hPa");
  
  DEBUG_STREAM.print("Air quality: ");
  DEBUG_STREAM.print(airValue);
	DEBUG_STREAM.println(" Analog (0-1023)");
}