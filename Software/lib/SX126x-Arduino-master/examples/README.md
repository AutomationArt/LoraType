# Examples

## Structure
All examples are available for ArduinoIDE and PlatformIO.    
The folders ending in ...Pio are for PlatformIO, other folders are for ArduinoIDE.    

## PingPong
This is the simplest example to show the basic communication between two devices over LoRa.    
This example just sends a data package (PING) and waits for a response (PONG) from another LoRa node.

## LoRaWan
This example shows the basics how to setup a LoRaWan compatible LoRa node.    
It is way more complex than the PingPong example and I strongly suggest you start to understand the basics of LoRa first with the PingPong example.    
In addition you should make yourself familiar with the way LoRaWan works.

## LoRaWan-Test-Multiple-Regions
This example is a more advanced example how to setup a LoRaWan compatible LoRa node.    
In addition it implements a BLE API to configure the LoRaWAN credentials over BLE with an Android application. The LoRaWAN credentials do not need to be hard-coded. Instead they are saved in flash memory after setup over BLE.
It is way more complex than the other examples and I strongly suggest you start to understand the basics of LoRaWAN first with the LoRaWAN example.    
In addition you should make yourself familiar with the way LoRaWan works.

## DeepSleep
This is a simple example for a receiver node that is optimized for lower power consumption.   
It is written for the ESP32 and is not tested on the ESP8266 nor on the nRF52.    
It uses the DIO1 interrupt output of the SX126x to wake the ESP32 from deep sleep once a LoRa package was received by the SX126x.    
In addition the example uses the **`Radio.SetRxDutyCycle()`** function which keeps the SX126x most of the time sleeping and wakes it only up for short periods of time to check for incoming data packets.

## DeepSleep-nRF52
This is a LoRaWan example for the nRF52 with minimized power consumption. It is thought as an example how to put the nRF52 into sleep mode. Read the [Low power example](Low_Power_Example.md) to find out more    

## Sensor-Gateway-Deepsleep
This example shows how to build a LoRa P2P based network with LoRa P2P sensor nodes and an ESP32 that acts as a gateway between the sensor nodes and an MQTT server. It receives the sensor data over LoRa and forwards them over WiFi to the MQTT server.