SX126x-Mesh-Network
===    
## Scope of the tutorial:
This tutorial will just create a simple Mesh network that sends randomly messages to other nodes within the Mesh network. There is no data structure or gateway functions to other networks like GSM or WiFi.
The goal here is to have a working core Mesh network that can be used to build up a second layer of communication which can be anything you want to do, e.g. a network of sensors that send out their sensor data.

## Target hardware:
The code is written to work on either an ESP32 or a nRF52 microcontroller and is not compatible with other microcontrollers. The LoRa transceivers used are [Semtech SX1262 transceivers](https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262). They are either connected to the microcontroller as a Adafruit Feather compatible breakout ([Circuitrocks Alora RFM1262](https://circuit.rocks/product:2685)) or integrated into a SOC together with a Nordic nRF52 ([Insight ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520)).

## And another important thing:
This code is NOT written to work with the ArduinoIDE. It is structured to be compiled under [PlatformIO](https://platformio.org/) using one of the IDE’s supported by PlatformIO. My favorite is Visual Studio Code.

## Tutorial for this example
A fairly deep explanation how this software works can be found in the [Alora RFM162 – LoRa Mesh Network](https://learn.circuit.rocks/alora-rfm162-lora-mesh-network) article

## How to compile for the different platforms
Have a look into the platformio.ini file. You can see examples for 5 different target boards, 2 nRF52 based boards and 3 ESP32 boards from different sources.

The code depends heavily on compiler defines to adapt different LoRa transceiver boards, used GPIO's, used libraries and available peripherals (display). These defines are set in the platformio.ini file.
Here is a list of the important defines:    
- -DADAFRUIT=1
  - Select code specific for the Adafruit nRF52832 Feather board
- -DHAS_DISPLAY=1
  - Set for  that have an OLED display based on the SH1106 chipset. This option requires to set as well
  - -DOLED_SDA=23
    - defines the GPIO used to connect the SDA
  - -DOLED_SCL=22
    - defines the GPIO used to connect the SCL
- -DRED_ESP=1
  - Used to select a specific setup for an Elecrow ESP32S WIFI BLE Board v1.0 board
- -DIS_WROVER=1
  - Used to distinguish between the ESP32 Wrover board and other ESP32 boards

Other defines used in the code, but setup by the PlatformIO packages
- ISP4520
  - Set for Insight ISP4520 boards
- ESP32
  - Set for ESP32 boards
- NRF52
  - Set for nRF52 boards

## Debug output over BLE
  In the application the BLE UART is activated and the boards starts advertising. On a nRF52 based board there is a DFU service, the OTA update service of Nordic to update the firmware on the chip. On all boards there is a simple BLE-UART service to send debug messages over BLE to a BLE-UART app like the [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) for Android    

## Library Dependencies
#### [SX126x-Arduino](https://github.com/beegee-tokyo/SX126x-Arduino)
- Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, ESP8266 and nRF52832. It will not work with other uC's like AVR.    

#### [esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- Driver for the SSD1306 and SH1106 based 128x64, 128x32, 64x48 pixel OLED display running on ESP8266/ESP32

## Platform Dependencies
#### [Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino)
- The Arduino BSP for Adafruit Bluefruit nRF52 series

#### [arduino-esp32](https://github.com/espressif/arduino-esp32)
- The Arduino BSP for Espressif ESP32 based board.

#### [Arduino Core for Circuitrocks Alora Boards](https://github.com/beegee-tokyo/Circuitrocks_ISP4520_Arduino)
- The Arduino BSP for the ISP4520 boards. Manual installation of the BSP is required as explained [here](https://github.com/beegee-tokyo/Circuitrocks_ISP4520_Arduino#bsp-installation)