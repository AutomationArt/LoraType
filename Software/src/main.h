#include <Arduino.h>
#include <WiFi.h>
#include "HttpsOTAUpdate.h"
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

//Default settings
#define LORAFREQ 868370000
#define LORATXPOWER 14
#define LORASPREAD 11
#define LORACODINGRATE 1
#define LORABAND 2


#define minLenghtOutMessage 4 //symbols

#define APIKEY "7f59d82c-f8a1-4e49-bb23-696b26a8ffcd"               
#define PINGADR "95.216.56.89"
#define FIRMVERS "2.0.5"

#define selfBroadMessaging  //for self-testing       

#define COLORED 0
#define UNCOLORED 1
extern uint32_t deviceID;


bool initLoRa(void);				   // start
int drawUpdate();					   // Print to display
int drawAbout();					   // Add about information
int drawSetOpMode(uint8_t, int16_t);   // Draw information about the selected communication quality
int drawSetFreq(String);			   // Draw the CPU frequency settings
int chatSingleDraw(uint32_t, uint8_t); // Print single Chat 1:1
int chatGenDraw(uint8_t);			   // Chat where is all users in mesh
int chatDrawOutmess();				   // Draw a string with a message to send in chat
int loraBandtoNum(uint16_t,bool);
int loraSendBroadcast(String);		   // Send message to all
int loraSendMessage(uint32_t, String); // Send message to user
int loraSendConfirm(uint32_t, String);
int MenuDraw();																 // Draw all menu
int MenuAllUserDraw(uint8_t, uint8_t);										 // Display a list of users who are in a mesh network
int MenuDrawArow(uint8_t);													 // Draw arrow in all menu
int MenuDrawLoraSet();														 // Draw lora settings
int MenuDrawAllSet();														 // Draw all settings
int MenuDrawDistance();														 // Draw graph of the relative distance to users by signal quality
int MenuDrawStatDb();														 //?????  In-memory message statistics
int MenuHistory(uint8_t);													 // History menu to understand where we are and what happened before
int MenuAllTagsDraw();														 // Display of all tags that are collected on the network
int MenuHeader(String);														 // Top E-ink header
int MenuNow();																 // Current menu item
int db_addTag(String);														 // Adding tags to the structure
int db_printAllTags();														 // Output to the logs of all tags that are stored
int db_printGenALL();														 // Output to the logs of all messages in general chat that are stored
int db_printIndALL();														 // Output to the logs of all individual chat that are stored
int db_addGenMessage(uint32_t, String, uint16_t, int16_t, int8_t);			 // Add message to structure-database
int db_addIndMessage(uint32_t, uint32_t, String, uint16_t, int16_t, int8_t); // Return the currently pressed button
int db_getCountInputMessages(uint32_t, bool);
void OnLoraData(uint32_t, uint8_t, uint16_t, int16_t, int8_t); // Callback message on LORA
void onNodesListChange(void);
String getLoraQualitySignal(int16_t, int16_t); // Callback when Node list Changed every 30 sec
String getBattery();						   // Get the battery charge as a percentage
String getVoltage();						   // Get the battery voltage
String getConfirmCode(String);				   // Confirmation code processing (in text)
int loraSendConfirm(uint32_t, String);		   // Sending a confirmation in individual chat
int tagSingleDraw(String, uint8_t);
int symCheck(byte);						 // Check the pressed button for the relation to the service buttons
int PrintFreeHeap();					 // Print free heap in device
int alertWindow(const char, const char); // alert window, without windows, of course.
bool CounterUsersUpdater();				 // Print counter all users in mesh
void onUpdateProgress(uint8_t, uint8_t); // Firmware upgrade process