#include <Arduino.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <Ticker.h>
#include <Log/my-log.h>

#include <SX126x-Arduino.h>
#include <SPI.h>
#include <Mesh/mesh.h>

#include "Tca8418Keyboard.h"
#include "backg.h"
#include "DEPG0150BxS810FxX_BW.h"
#include "e_ink_display.h"
#include "esp32-hal-cpu.h"
#include <Preferences.h>
#include "otadrive_esp.h"
#include "ESP32Ping.h"

/** LoRa package types */
#define LORA_INVALID 0
#define LORA_DIRECT 1
#define LORA_FORWARD 2
#define LORA_BROADCAST 3
#define LORA_NODEMAP 4

//#define logc  //all Serial print enable
#define PinSDA 21
#define PinSCL 22
#define PinBattery 35
#define PinLed 33

extern uint32_t deviceID;

bool initLoRa(void);

// Display
void initDisplay(void);
void dispWriteHeader(void);
void dispWrite(String text, int x, int y);
void dispUpdate(void);