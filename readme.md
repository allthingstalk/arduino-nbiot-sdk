# arduino-nbiot-sdk

This is a SDK by AllThingsTalk that provides connectivity to their cloud through NB-IoT radios.

## Hardware

This SDK has been tested to work with the following hardware
- [Sodaq Mbili](http://support.sodaq.com/sodaq-one/sodaq-mbili-1284p/) with ublox NB-IoT bee

## Installation

Download the source code and copy the content of the zip file to your arduino libraries folder (usually found at /libraries) _or_ import the .zip file directly using the Arduino IDE.

## Sending data

### Single asset

Send a single datapoint to a single asset using the `send(value, asset)` function. Value can be any primitive type `integer`, `float`, `boolean` or `String`. For example

```
nbiot.send(25, "counter");
nbiot.send(false, "motion");
```

### Binary payload

Using the [AllThingsTalk ABCL language](http://docs.allthingstalk.com/developers/custom-payload-conversion/), you can send a binary string containing datapoints of multiple assets in a single message. The example below show how you can easily construct and send your own custom payload.

> Make sure you set the correct decoding file at AllThingsTalk. Please check our documentation and the included experiments for examples.

```
  payload.reset();

  payload.addInteger(25);
  payload.addNumber(false);
  payload.addNumber(3.1415926);

  payload.send(false);
```

## Examples

### Basic example

* `counter.ino`

### Rapid Development Kit experiments

* `count-visits.ino` Count the amount of visits to a room for facility management
* `environmental-sensing.ino` Measure your surrounding
* `guard-your-stuff.ino` Track an object when it starts to move