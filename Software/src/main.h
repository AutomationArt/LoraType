#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Ticker.h>
#include <Log/my-log.h>     
#include "esp_log.h"    
#include <SX126x-Arduino.h>
#include <SPI.h>
#include <Mesh/mesh.h>
#include <Adafruit_TCA8418.h> //keyboard library
#include "backg.h" //lifehack 
#include "DEPG0150BxS810FxX_BW.h" //library Heltec E-Ink
#include "e_ink_display.h"
#include "esp32-hal-cpu.h"
#include <Preferences.h>
#include "otadrive_esp.h"
#include "ESP32Ping.h"

//LoRa package types
#define LORA_INVALID 0 
#define LORA_DIRECT 1
#define LORA_FORWARD 2
#define LORA_BROADCAST 3
#define LORA_NODEMAP 4


#define PinSDA 21
#define PinSCL 22
#define PinBattery 35
#define PinLed 33
#define PinIRQKeypad 34

#define inkRST_PIN 32
#define inkDC_PIN 12
#define inkCS_PIN 15
#define inkBUSY_PIN 27
#define inkCLK_PIN 14


#define minLenghtOutMessage 5 //symbols

#define APIKEY "*****************************"
#define PINGADR "95.216.56.89"
#define FIRMVERS "2.0.3"

#define selfBroadMessaging

#define COLORED 0
#define UNCOLORED 1

extern uint32_t deviceID;

