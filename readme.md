# arduino-nbiot-sdk

This is a SDK by AllThingsTalk that provides connectivity to their cloud through NB-IoT radios.

## Hardware

This library has been developed for:

* [Sodaq Mbili](http://support.sodaq.com/sodaq-one/sodaq-mbili-1284p/)
* [Sodaq UBEE](http://support.sodaq.com/sodaq-one/ubee/) (ublox SARA-N211 nb-iot radio)

## Installation

Download the source code and copy the content of the zip file to your arduino libraries folder (usually found at /libraries) _or_ import the .zip file directly using the Arduino IDE.

## Sending data

### Setting keys

Open the [`keys.h`](https://github.com/allthingstalk/arduino-nbiot-sdk/blob/master/keys.h) file on your computer and enter your _deviceid_ and _devicetoken_ of the arduino-nbiot-sdk. You can find these credentials under your device at AllThingsTalk in the _SETTINGS > Authentication_ tab.

### Payloads

There are three ways to send your data to AllThingsTalk

* `Standard json`
* `Cbor payload`
* `Binary payload`

Standard json will send a single datapoint to a single asset. Both _Cbor_ and _Binary_ allow you to construct your own payload. The former is slightly larger in size, the latter requires a small decoding file [(example)](https://github.com/allthingstalk/arduino-nbiot-sdk/blob/master/arduino-nbiot-sdk/examples/environmental-sensing/nbiot-environmental-sensing-payload-definition.json) on the receiving end.

#### Single asset

Send a single datapoint to a single asset using the `send(value, asset)` functions. Value can be any primitive type `integer`, `float`, `boolean` or `String`. For example

```
ATT_NBIOT nbiot;
```
```
  nbiot.send(25, "counter");
  nbiot.send(false, "motion");
```

#### Cbor

```
ATT_NBIOT nbiot;
CborBuilder payload(nbiot);  // Construct a payload object
```
```
  payload.reset();
  payload.map(1);  // Set number of datapoints in payload
  payload.addInteger(25, "counter");
  payload.send();
```

#### Binary payload

Using the [AllThingsTalk ABCL language](http://docs.allthingstalk.com/developers/custom-payload-conversion/), you can send a binary string containing datapoints of multiple assets in a single message. The example below shows how you can easily construct and send your own custom payload.

> Make sure you set the correct decoding file at AllThingsTalk. Please check our documentation and the included experiments for examples.

```
ATT_NBIOT nbiot;
PayloadBuilder payload(nbiot);  // Construct a payload object
```
```
  payload.reset();
  payload.addInteger(25);
  payload.addNumber(false);
  payload.addNumber(3.1415926);
  payload.send();
```

### Examples

Basic example showing all fundamental parts to set up a working example. Send data from the device, over NB-IoT to AllThingsTalk.

* `counter` This example shows how you can send over a simple integer counter using either _json_, _cbor_ or a _binary payload_.

Simply uncomment your selected method for sending data at the top of the sketch.

```
// Uncomment your selected methd for sending data
#define JSON
//#define CBOR
//#define BINARY
```