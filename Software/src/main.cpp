/*
LORATYPE...|
https://fosstodon.org/@loratype
https://lora.readthedocs.io/en/latest
*/

#include "main.h"
#include "ledFunction.h"

bool stateUpdate = false;
Preferences pref;
AsyncWebServer ServerELEG(80);
unsigned char img[5000];
Paint pt(img, 0, 0);
Adafruit_TCA8418 keypad;
volatile bool TCA8418_event = false;
bool ledIndicate = false;

void TCA8418_irq()
{
	TCA8418_event = true;
}

enum keyServ
{
	//    3 7 - ctrl
	//    2 3 - space
	//    4 7 - Enter
	//    3 3 - left
	//    3 1 - right
	//    4 4 - Menu
	//    4 5 - backspace
	// 	  3 4 - UP
	// 	  2 0 - down
	//    3 0 - OK

	keyCTRL = 37,
	keySPACE = 23,
	keyENTER = 47,
	keyLEFT = 33,
	keyRIGHT = 31,
	keyMENU = 44,
	keyBACKSPACE = 45,
	keyUP = 34,
	keyDOWN = 20,
	keyOKCenter = 30,
};

char keyDownCase[5][8] = {
	{'i', 'u', 'y', 't', 'r', 'e', 'w', 'q'},
	{'k', 'j', 'h', 'g', 'f', 'd', 's', 'a'},
	{'~', 'b', 'v', ' ', 'c', 'x', 'z', 'n'},
	{'~', '~', 'o', '~', '~', 'l', 'm', '~'},
	{'~', '~', '~', '~', '~', '~', 'p', '~'},
};

char keySymbCase[5][8] = {
	{'-', '_', ')', '(', '3', '2', '1', '#'},
	{'\'', ';', ':', '/', '6', '5', '4', '*'},
	{'~', '!', '?', '0', '9', '8', '7', ','},
	{'~', '~', '~', '~', '~', '$', '.', '~'},
	{'~', '~', '~', '~', '~', '~', '@', '~'},
};

DEPG0150BxS810FxX_BW epd154bw(inkRST_PIN, inkDC_PIN, inkCS_PIN, inkBUSY_PIN, inkCLK_PIN);

dataMsg outData;
nodesList routeToNode;
uint32_t nodeId[48], firstHop[48];
uint8_t numHops[48], numElements;
uint8_t messFrame = 0, userFrame = 0;
boolean nodesListChanged = false;
time_t sendRandom, itwork, sendBrodcast;
bool workFlag = 1;
uint32_t getActiveUser = 0;
String getActiveTag = "";
uint16_t allstrCounter = 0;
char sym;
String outMessage = "";
bool OnLoraFlag = false;
uint32_t OnLora_rxfromID;
String OnLora_rxPayload;
uint16_t OnLora_rxSize;
int16_t OnLora_rxRssi;
int8_t OnLora_rxSnr;
String selfText[17] = {	
	"He who does not take risks lives on his pension.",
	"They're all literate, aren't they? They all have sensory deprivation.",
	"It's very hard to lose a dream, even by pursuing it.",
	"And it was scary and embarrassing.",
	"Freedom is a conscious necessity.",
	"You could save everyone by saving yourself. In a spiritual sense, of course.",
	"But I have never felt sorry and never envied anyone.",
	"Better bitter happiness than a gray, dismal life.",
	"Happiness for free and for all, let no one walk away offended.",
	"Get out on your own. I ain't got nothing to do with it, man.",
	"He got famous, and now three people knew he existed.",
	"It takes money for a man to stop thinking about it.",
	"Let the Zone bury its own dead, you know that law.",
	"What good is your knowledge? Whose conscience will they make sick?",
	"A bolt is power, a bolt is an argument!",
	"In the Zone, the straight path is not the shortest.",
	"How can you be happy at the expense of others' misfortune?"};
uint8_t y_dis, counterResMess = 0;
bool keyboardUpperFlag = false;
struct menuitem
{
	bool active;
	uint8_t itemNum;
	const char *itemName;
};
menuitem menuAll[] =
	{
		{false, 0, "Everything chat"},
		{false, 1, "Users Online"},
		{false, 2, "Friend Quality"},
		{false, 3, "Lora Settings"},
		{false, 4, "All Settings"},
		{false, 5, "Update firmware (OTA)"},
		{false, 6, "About"}};

enum menuList
{
	MAINMENU = -1,
	EVERYTHING,
	USERS,
	FQA,
	LORASETTINGS,
	ALLSETTINGS,
	UPDATE,
	ABOUT,
};
byte menuCount = sizeof(menuAll) / sizeof(menuAll[0]); // global count in main menu
const uint8_t countMessage = 30;
const uint16_t countTags = 300;
const uint16_t countinskStr = 100;
// General Chat
struct chat
{
	uint32_t OnLora_fromID;
	uint16_t OnLora_rxSize;
	int16_t OnLora_rxRssi;
	int8_t OnLora_rxSnr;
	String message;
} GenChat[countMessage];

// Individual Chat
struct indchat
{
	uint32_t OnLora_fromID;
	uint32_t OnLora_toID;
	uint16_t OnLora_rxSize;
	int16_t OnLora_rxRssi;
	int8_t OnLora_rxSnr;
	String message;
	bool confirm;
	String confirmCode;
} IndChat[countMessage];

struct tags
{
	int8_t active;
	String tag;
	uint16_t counter;
} AllTag[countTags];

struct distanceNode
{
	unsigned long id;
	int16_t rssi;
	int16_t snr;
};

distanceNode dnode[48];

int8_t menuHis[5], menuNow = -1;
bool isIndChat = false, isGenChat = false, isTagsView = false, isCtrlActive = false;
byte menuNowSelect = 0;

int alertWindow(const char *head, const char *message)
{
	pt.DrawRectangle(20, 55, 180, 100, COLORED);
	pt.DrawFilledRectangle(21, 56, 179, 99, UNCOLORED);
	pt.DrawStringAt(27, 65, head, &Font12, COLORED);
	pt.DrawStringAt(27, 80, message, &Font12, COLORED);

	return 0;
}

String getLoraQualitySignal(int16_t SNR, int16_t RSSI)
{
	//((SNR - SNR_min) / (SNR_max - SNR_min) + (RSSI - RSSI_min) / (RSSI_max - RSSI_min)) / 2 * 100
	// If RSSI=-30dBm: signal is strong.
	// If RSSI=-120dBm: signal is weak.
	// RSSI min = -130
	// RSSI max = -20
	// SNR min = -20
	// SNR max = 30

	// ((SNR + 20) / 50 + (RSSI + 130) / 110) / 2 * 100)
	ESP_LOGD("LORA", "%d %d", SNR, RSSI);
	return String((((SNR + 20) / 50) + ((RSSI + 130) / 110)) / 2);
}

int drawBatteryState(int batValue)
{
	pt.DrawRectangle(153, 4, 155, 7, UNCOLORED);
	pt.DrawRectangle(155, 2, 170, 9, UNCOLORED);

	for (int i = map(batValue, 0, 100, 0, 14); i > 0; i = i - 2)
	{

		pt.DrawVerticalLine(170 - i, 4, 4, UNCOLORED);
	}
	return 0;
}

int drawSetFreq(String ffrq = "")
{
	pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);
	String tempText = "Enter Frequency: " + ffrq;
	pt.DrawStringAt(10, 30, tempText.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 45, "- 850~930 (Example: 86837 = 868.37 MHz)", &Font8, COLORED);
	return 0;
}

int drawSetOpMode(uint8_t mode_set, int16_t data_set[7][6])
{
	String tempTextSet;
	pt.DrawFilledRectangle(10, 90, 200, 200, UNCOLORED);

	if (mode_set == 0)
	{
		pt.DrawFilledRectangle(10, 180, 200, 195, COLORED);
		tempTextSet = "(Max range, slow data rate)";
		pt.DrawStringAt(10, 182, tempTextSet.c_str(), &Font12, UNCOLORED);
	}
	else if (mode_set == 6)
	{
		pt.DrawFilledRectangle(10, 180, 200, 195, COLORED);
		tempTextSet = "(Min range,fast DR,min.battery impact)";
		pt.DrawStringAt(10, 180, tempTextSet.c_str(), &Font12, UNCOLORED);
		// pt.DrawStringAt(10, 105, "- (14-22dBm. >14dBm may be illegal)", &Font8, COLORED);
	}

	tempTextSet = "Select mode: <- " + String(mode_set) + " ->";
	pt.DrawStringAt(10, 90, tempTextSet.c_str(), &Font12, COLORED);

	tempTextSet = "BandWidth: " + String(loraBandtoNum(data_set[mode_set][1], true)) + " kHz";
	pt.DrawStringAt(10, 105, tempTextSet.c_str(), &Font12, COLORED);

	tempTextSet = "Spreading factor: " + String(data_set[mode_set][3]);
	pt.DrawStringAt(10, 120, tempTextSet.c_str(), &Font12, COLORED);

	if (data_set[mode_set][2] == 1)
	{
		tempTextSet = "Coding rate: 4/5";
	}
	else
	{
		tempTextSet = "Coding rate: 2";
	}
	pt.DrawStringAt(10, 135, tempTextSet.c_str(), &Font12, COLORED);

	tempTextSet = "Sensitivity : " + String(data_set[mode_set][4]) + " dB";
	pt.DrawStringAt(10, 150, tempTextSet.c_str(), &Font12, COLORED);

	tempTextSet = "Transmission time: " + String(data_set[mode_set][5]) + " ms";
	pt.DrawStringAt(10, 165, tempTextSet.c_str(), &Font12, COLORED);

	return 0;
}

int drawAbout()
{
	pt.DrawFilledRectangle(0, 11, 200, 200, UNCOLORED);
	pt.DrawHorizontalLine(90, 25, 20, COLORED);
	pt.DrawLine(110, 25, 125, 40, COLORED);
	pt.DrawVerticalLine(125, 40, 15, COLORED);
	pt.DrawLine(125, 55, 110, 70, COLORED);
	pt.DrawHorizontalLine(90, 70, 20, COLORED);
	pt.DrawLine(90, 70, 75, 55, COLORED);
	pt.DrawVerticalLine(75, 40, 15, COLORED);
	pt.DrawLine(75, 40, 90, 25, COLORED);
	pt.DrawLine(90, 25, 110, 40, COLORED);
	pt.DrawVerticalLine(110, 25, 15, COLORED);
	pt.DrawLine(110, 40, 90, 55, COLORED);
	pt.DrawHorizontalLine(75, 55, 15, COLORED);
	pt.DrawLine(90, 55, 110, 70, COLORED);
	pt.DrawStringAt(20, 80, "https://Loratype.org", &Font12, COLORED);
	pt.DrawStringAt(3, 95, "Meet LoraType - the urban 'teletype", &Font8, COLORED);
	pt.DrawStringAt(3, 105, "tweeting'. It uses radio signal on", &Font8, COLORED);
	pt.DrawStringAt(3, 115, "free frequency.It uses radio signal", &Font8, COLORED);
	pt.DrawStringAt(3, 125, "on free frequency,which allows users", &Font8, COLORED);
	pt.DrawStringAt(3, 135, "to be independent of Int.connection,", &Font8, COLORED);
	pt.DrawStringAt(3, 145, "which is crucial in case of any ", &Font8, COLORED);
	pt.DrawStringAt(3, 155, "armageddon situation.", &Font8, COLORED);
	pt.DrawStringAt(3, 165, "It works like walkie talkie.", &Font8, COLORED);

	return 0;
}

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
ESP_LOGD("UpdateOTA", "OTA update started");	

}

void onOTAProgress(size_t current, size_t final)
{

	if (millis() - ota_progress_millis > 1000)
	{
		ota_progress_millis = millis();
		String result = "Progress: " + String(current / 1000) + " kb. Final:" + String(final / 1000) + " kb.";
		pt.DrawFilledRectangle(0, 50, 200, 200, UNCOLORED);
		pt.DrawStringAt(15, 60, result.c_str(), &Font8, COLORED);
		drawUpdate();
	}
}

void onOTAEnd(bool success)
{

	if (success)
	{
		pt.DrawStringAt(15, 70, "OTA update finished successfully!", &Font8, COLORED);
		ESP_LOGD("UpdateOTA", "OTA update finished successfully!");
	}
	else
	{
		ESP_LOGD("UpdateOTA", "There was an error during OTA update!");
		pt.DrawStringAt(15, 70, "There was an error during!", &Font8, COLORED);
	}

}

int OtaUpdate()
{
	String wifi_name = "", wifi_pass = "", tempText = "";
	y_dis = 20;
	MenuHeader("LoraType Update");

	char DeviceIDmac[9];	
	itoa(deviceID, DeviceIDmac, 16);
	String point = "LoraType-" + String(DeviceIDmac);
	String  pointwifi="Start Wi-Fi: "+point;

	pt.DrawStringAt(10, y_dis, pointwifi.c_str(), &Font8, COLORED);
	y_dis += 14;
	drawUpdate();

	
	WiFi.softAP(point.c_str(), "loratype");

	IPAddress IP = WiFi.softAPIP();
	y_dis += 10;
	String ip = "Server IP: " + IP.toString();
	pt.DrawStringAt(10, y_dis, ip.c_str(), &Font8, COLORED);
	drawUpdate();

	y_dis += 10;
	pt.DrawStringAt(10, y_dis, "OTA Server available", &Font8, COLORED);
	drawUpdate();

	uint8_t stage = 0;

	ServerELEG.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
				  { request->send(200, "text/plain", "Hello! This is ElegantOTA AsyncDemo in LoraType. Get to /upload"); });

	ElegantOTA.begin(&ServerELEG); 
	// ElegantOTA callbacks
	ElegantOTA.onStart(onOTAStart);
	ElegantOTA.onProgress(onOTAProgress);
	ElegantOTA.onEnd(onOTAEnd);

	ServerELEG.begin();

	while (1)
	{
		ElegantOTA.loop();
	}

	return 0;
}

int MenuAllTagsDraw()
{
	pt.DrawFilledRectangle(0, 23, 200, 200, UNCOLORED);

	uint8_t countTagsNow = 0;
	for (int idx = 0; idx <= countTags - 1; idx++)
	{
		if (AllTag[idx].tag != "")
		{
			countTagsNow++;
		}
	}

	ESP_LOGD("ALLTAGS", "%d", countTagsNow);

	menuCount = countTagsNow;
	String printtags;
	for (int idx = 0; idx <= 10; idx++)
	{
		printtags = AllTag[countTags - 1 - idx].tag;
		pt.DrawStringAt(25, 25 + (idx * 15), printtags.c_str(), &Font12, COLORED);
	}
	return 0;
}

int MenuHistory(int menunum) // menuNow global var
{
	for (uint8_t i = 0; i < 4; i++)
	{
		menuHis[i] = menuHis[i + 1];
	}
	menuHis[4] = menunum;
	menuNow = menunum;
	return 0;
}

int MenuNow()
{
	return menuNow;
}

int symCheck(byte keyCheck)
{
	//    3 7 - ctrl
	//    2 3 - space
	//    4 7 - Enter
	//    3 3 - left
	//    3 1 - right
	//    4 4 - Menu
	//    4 5 - backspace
	// 	  3 4 - UP
	// 	  2 0 - down
	//    3 0 - OK

	byte specKey[9] = {37, 47, 33, 31, 44, 45, 34, 20, 30};

	for (int i = 0; i <= 8; i++)
	{
		if (keyCheck == specKey[i])
		{
			return 1;
		}
	}
	return 0;
}

bool caseUpperFlag = false;
uint8_t keyboardColl = 0, keyboardRow = 0;

int loraBandtoNum(uint16_t band, bool direction = true)
{
	if (direction)
	{
		switch (band)
		{
		case 125:
			return 0;
			break;

		case 250:
			return 1;
			break;

		case 500:
			return 2;
			break;
		default:
			return 1;
			break;
		}
	}
	else
	{
		switch (band)
		{
		case 0:
			return 125;
			break;

		case 1:
			return 250;
			break;

		case 2:
			return 500;
			break;
		default:
			return 250;
			break;
		}
	}
}

int loraSendConfirm(uint32_t userMesh, String messKey)
{
	if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
	{
		numElements = numOfNodes();
		if (numOfNodes() >= 1)
		{
			getRoute(userMesh, &routeToNode);

			xSemaphoreGive(accessNodeList);
			outData.mark1 = 'L';
			outData.mark2 = 'o';
			outData.mark3 = 'R';
			if (routeToNode.firstHop != 0)
			{
				outData.dest = routeToNode.firstHop;
				outData.from = routeToNode.nodeId;
				outData.orig = deviceID;
				outData.type = LORA_FORWARD;
			}
			else
			{
				outData.dest = routeToNode.nodeId;
				outData.from = deviceID;
				outData.orig = deviceID;
				outData.type = LORA_DIRECT;
			}

			String msgData = "@" + messKey;
			uint16_t MsLen = msgData.length() + 1;
			msgData.getBytes(outData.data, MsLen);
			uint16_t OutLen = sizeof(outData.data) / sizeof(outData.data[0]);
			uint16_t dataLen = MAP_HEADER_SIZE + OutLen;

			if (!addSendRequest(&outData, dataLen))
			{

				ESP_LOGD("LORA", "Send CONFIRM fail");
			}
			else
			{
				ESP_LOGD("LORA", "Send CONFIRM - OK");
			}
		}
		else
		{
			xSemaphoreGive(accessNodeList);
			ESP_LOGD("LORA", "Not enough nodes in the list in CONFIRM");
		}
	}
	else
	{

		ESP_LOGD("LORA", "Could not access the nodes list in CONFIRM");
	}

	return 0;
}

int loraSendMessage(uint32_t userMesh, String msgData)
{
	if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
	{
		numElements = numOfNodes();
		if (numOfNodes() >= 1)
		{
			getRoute(userMesh, &routeToNode);
			xSemaphoreGive(accessNodeList);
			outData.mark1 = 'L';
			outData.mark2 = 'o';
			outData.mark3 = 'R';
			if (routeToNode.firstHop != 0)
			{
				outData.dest = routeToNode.firstHop;
				outData.from = routeToNode.nodeId;
				outData.orig = deviceID;
				outData.type = LORA_FORWARD;

				ESP_LOGD("LORA", "Queuing msg to hop to %08X over %08X", outData.from, outData.dest);
			}
			else
			{
				outData.dest = routeToNode.nodeId;
				outData.from = deviceID;
				outData.orig = deviceID;
				outData.type = LORA_DIRECT;

				ESP_LOGD("LORA", "Queuing msg direct to %08X", outData.dest);
			}

			String key = String(deviceID);

			char DeviceIDmac[9];
			itoa(deviceID, DeviceIDmac, 16);
			String output = String(DeviceIDmac);

			String messKey = output.substring(3, 7) + random(10000, 90000);

			msgData = "~" + messKey + "~" + msgData;
			uint16_t MsLen = msgData.length() + 1;
			msgData.getBytes(outData.data, MsLen);
			uint16_t OutLen = sizeof(outData.data) / sizeof(outData.data[0]);
			uint16_t dataLen = MAP_HEADER_SIZE + OutLen;

			if (!addSendRequest(&outData, dataLen))
			{
				LedLoraWarning(false);
				ESP_LOGD("LORA", "Send - fail");
			}
			else
			{
				ESP_LOGD("LORA", "Send - Ok");
				db_addIndMessage(13, userMesh, msgData, 0, 0, 0);
				LedLoraWarning(true);
			}
		}
		else
		{
			xSemaphoreGive(accessNodeList);
			ESP_LOGD("LORA", "Not enough nodes in the list");
		}
	}
	else
	{
		ESP_LOGD("LORA", "Could not access the nodes list");
	}
	return 0;
}

int loraSendBroadcast(String BroadMessage)
{
	ESP_LOGD("LORA", "Send broadcast");
	outData.mark1 = 'L';
	outData.mark2 = 'o';
	outData.mark3 = 'R';
	getNextBroadcastID();

	outData.dest = getNextBroadcastID();
	outData.from = deviceID;
	outData.type = LORA_BROADCAST;

	uint16_t MsLen = BroadMessage.length();
	BroadMessage.getBytes(outData.data, MsLen + 1);
	uint16_t OutLen = sizeof(outData.data) / sizeof(outData.data[0]);

	uint16_t dataLen = MAP_HEADER_SIZE + OutLen;
	if (!addSendRequest(&outData, dataLen))
	{
		LedLoraWarning(false);
		ESP_LOGD("LORA", "Sending BR fail");
		return 1;
	}
	else
	{
		ESP_LOGD("LORA", "Sending BR - OK");
		db_addGenMessage(1, BroadMessage, 0, 0, 0);
		LedLoraWarning(true);
		return 0;
	}
}

void OnLoraData(uint32_t fromID, uint8_t *rxPayload, uint16_t rxSize, int16_t rxRssi, int8_t rxSnr)
{
	OnLoraFlag = true;
	OnLora_rxfromID = fromID;
	OnLora_rxPayload = String((char *)rxPayload);
	OnLora_rxSize = rxSize;
	OnLora_rxRssi = rxRssi;
	OnLora_rxSnr = rxSnr;
}

int db_printAllTags()
{
	for (uint16_t g = countTags; g >= 150; g--)
	{
		ESP_LOGD("DATABASE", "%d : %d - %d - %s", g, AllTag[g].active, AllTag[g].counter, AllTag[g].tag);
	}
	return 0;
}

int db_printGenALL()
{
	for (int g = 1000; g >= 980; g--)
	{
		ESP_LOGD("DATABASE", "%d: %d - %s - %d - %d - %d", g, GenChat[g].OnLora_fromID, GenChat[g].message, GenChat[g].OnLora_rxRssi, GenChat[g].OnLora_rxSize, GenChat[g].OnLora_rxSnr);
	}
	return 0;
}

int db_printIndALL()
{
	for (uint8_t g = 30; g >= 0; g--)
	{
		ESP_LOGD("DATABASE", "%d: %d - %d - %s - %d - %d - %d - %d - %d", g, IndChat[g].OnLora_fromID,
				 IndChat[g].OnLora_toID, IndChat[g].message,
				 IndChat[g].OnLora_rxRssi, IndChat[g].OnLora_rxSize,
				 IndChat[g].OnLora_rxSnr, (int)IndChat[g].confirm,
				 IndChat[g].confirmCode);
	}
	return 0;
}

int db_addTag(String tagNow)
{
	for (uint8_t i = 0; i < countTags - 1; i++)
	{
		if (tagNow == AllTag[i].tag)
		{
			ESP_LOGD("LORA", "Tag already exist");
			return 0;
		}
	}
	for (uint8_t i = 0; i < countTags - 1; i++)
	{
		AllTag[i].active = AllTag[i + 1].active;
		AllTag[i].counter = AllTag[i + 1].counter;
		AllTag[i].tag = AllTag[i + 1].tag;
	}
	AllTag[countTags - 1].tag = tagNow;
	ESP_LOGD("LORA", "Tag added");

	return 0;
}

int db_addGenMessage(uint32_t dbfromID, String dbrxPayload, uint16_t dbrxSize, int16_t dbxRssi, int8_t dbrxSnr)
{
	for (uint8_t i = 0; i < countMessage - 1; i++)
	{
		GenChat[i].OnLora_fromID = GenChat[i + 1].OnLora_fromID;
		GenChat[i].OnLora_rxRssi = GenChat[i + 1].OnLora_rxRssi;
		GenChat[i].OnLora_rxSize = GenChat[i + 1].OnLora_rxSize;
		GenChat[i].OnLora_rxSnr = GenChat[i + 1].OnLora_rxSnr;
		GenChat[i].message = GenChat[i + 1].message;
	}

	GenChat[countMessage - 1].OnLora_fromID = dbfromID;
	GenChat[countMessage - 1].OnLora_rxRssi = dbxRssi;
	GenChat[countMessage - 1].OnLora_rxSnr = dbrxSnr;
	GenChat[countMessage - 1].message = dbrxPayload;

	ESP_LOGD("LORA", "Message added");

	uint8_t spacepos = dbrxPayload.indexOf(" ");
	if (spacepos != -1 && dbrxPayload.charAt(0) == '#')
	{
		db_addTag(dbrxPayload.substring(0, spacepos));
		db_printAllTags();
	}

	return 0;
}

int db_addIndMessage(uint32_t dbfromID, uint32_t dbtoID, String dbrxPayload, uint16_t dbrxSize, int16_t dbxRssi, int8_t dbrxSnr)
{

	for (int i = 0; i < countMessage - 1; i++)
	{
		IndChat[i].OnLora_fromID = IndChat[i + 1].OnLora_fromID;
		IndChat[i].OnLora_toID = IndChat[i + 1].OnLora_toID;
		IndChat[i].OnLora_rxRssi = IndChat[i + 1].OnLora_rxRssi;
		IndChat[i].OnLora_rxSize = IndChat[i + 1].OnLora_rxSize;
		IndChat[i].OnLora_rxSnr = IndChat[i + 1].OnLora_rxSnr;
		IndChat[i].message = IndChat[i + 1].message;
		IndChat[i].confirm = IndChat[i + 1].confirm;
		IndChat[i].confirmCode = IndChat[i + 1].confirmCode;
	}

	String codeNow = getConfirmCode(dbrxPayload);
	IndChat[countMessage - 1].confirmCode = codeNow;

	dbrxPayload.remove(0, 1);
	dbrxPayload.remove(0, dbrxPayload.indexOf("~"));
	dbrxPayload.remove(0, 1);

	IndChat[countMessage - 1].confirm = false;
	IndChat[countMessage - 1].OnLora_fromID = dbfromID;
	IndChat[countMessage - 1].OnLora_toID = dbtoID;
	IndChat[countMessage - 1].OnLora_rxRssi = dbxRssi;
	IndChat[countMessage - 1].OnLora_rxSnr = dbrxSnr;
	IndChat[countMessage - 1].message = dbrxPayload;

	ESP_LOGD("LORA", "Message (individual) added");
	// db_printIndALL();

	return 0;
}

String getConfirmCode(String Payload)
{
	int sep = Payload.indexOf("~");
	String Data = Payload.substring(sep + 1, Payload.length() - 1);
	int sept = Data.indexOf("~");
	Data = Payload.substring(0, sept + 1);
	Data.remove(0, 1);
	return Data;
}

void onNodesListChange(void)
{
	nodesListChanged = true;
}

int MenuDraw()
{
	pt.DrawFilledRectangle(0, 11, 200, 200, UNCOLORED);
	for (int i = 0; i <= menuCount - 1; i++)
	{
		pt.DrawStringAt(20, 25 + (i * 15), menuAll[i].itemName, &Font12, COLORED);
	}
	char DeviceIDmac[9];
	itoa(deviceID, DeviceIDmac, 16);
	String MyNode = "My m.id:" + String(DeviceIDmac) + " / Ver:" + String(FIRMVERS);
	pt.DrawStringAt(20, 200 - 35, MyNode.c_str(), &Font8, COLORED);
	return 0;
}

int MenuDrawArow(uint8_t selected)
{

	pt.DrawFilledRectangle(0, 24, 19, 200, UNCOLORED);
	pt.DrawStringAt(3, 25 + (selected * 15), "->", &Font12, COLORED);
	ESP_LOGD("DRAW", "Draw arrow");
	return 0;
}

int MenuDrawDistance()
{
	// https://sensing-labs.com/f-a-q/a-good-radio-level/

	pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);

	pt.DrawStringAt(20, 25, "(No real-time updates yet)", &Font8, COLORED);
	pt.DrawVerticalLine(10, 25, 160, COLORED);
	pt.DrawHorizontalLine(10, 185, 170, COLORED);

	pt.DrawFilledCircle(10, 185, 7, COLORED);
	pt.DrawCircle(10, 185, 9, COLORED);
	pt.DrawCircle(10, 185, 12, COLORED);

	pt.DrawStringAt(3, 60, "S", &Font8, COLORED);
	pt.DrawStringAt(3, 69, "N", &Font8, COLORED);
	pt.DrawStringAt(3, 78, "R", &Font8, COLORED);

	pt.DrawStringAt(100, 190, "RSSI", &Font8, COLORED);
	char line[10];
	uint8_t xran = 0, yran = 0;

	for (int g = 0; g < numElements; g++)
	{
		sprintf(line, "%02X%02X%02X", (uint8_t)(nodeId[g] >> 24), (uint8_t)(nodeId[g] >> 16), (uint8_t)(nodeId[g] >> 8));

		if (abs(dnode[g].rssi) < 115 && abs(dnode[g].snr) < 7) // good
		{
			xran = random(10, 100);
			yran = random(100, 170);

			pt.DrawStringAt(xran + 7, yran - 7, line, &Font8, COLORED);
			pt.DrawCircle(xran, yran, 6, COLORED);
		}
		else if ((abs(dnode[g].rssi) >= 115 && abs(dnode[g].rssi) <= 126 && abs(dnode[g].snr) <= 7) || (abs(dnode[g].rssi) <= 115 && abs(dnode[g].snr) >= 7 && abs(dnode[g].snr) <= 15)) // fair
		{
			xran = random(10, 140);
			yran = random(60, 100);
			pt.DrawStringAt(xran + 7, yran - 7, line, &Font8, COLORED);
			pt.DrawCircle(xran, yran, 4, COLORED);
		}
		else if ((abs(dnode[g].rssi) <= 126 && abs(dnode[g].snr) > 15) || (abs(dnode[g].rssi) > 126 && abs(dnode[g].snr) < 15)) // bad
		{
			xran = random(150, 170);
			yran = random(30, 150);
			pt.DrawStringAt(xran + 7, yran - 7, line, &Font8, COLORED);
			pt.DrawCircle(xran, yran, 2, COLORED);
		}
		else // error
		{
			pt.DrawCircle(random(15, 170), random(30, 185), 1, COLORED);
		}
	}
	return 0;
}

int MenuDrawAllSet()
{
	if (pref.begin("AllSettings", false))
	{
		pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);
		String tmpPrint = "CPU Frequency: " + String(pref.getInt("esp_freq", 80)) + " mHz";
		pt.DrawStringAt(10, 30, tmpPrint.c_str(), &Font12, COLORED);
		bool legal = pref.getBool("legality", true);
		if (legal)
		{
			tmpPrint = "Legality: Yes";
		}
		else
		{
			tmpPrint = "Legality: No";
		}

		pt.DrawStringAt(10, 45, tmpPrint.c_str(), &Font12, COLORED);
		if (legal == false)
		{
			pt.DrawFilledRectangle(10, 60, 190, 82, COLORED);
			pt.DrawStringAt(20, 62, "(You have some restrictions on", &Font8, UNCOLORED);
			pt.DrawStringAt(20, 72, "the airtime for sending messages)", &Font8, UNCOLORED);
		}
		else
		{
			pt.DrawFilledRectangle(10, 60, 190, 82, COLORED);
			pt.DrawStringAt(20, 62, "(By doing so, you may", &Font8, UNCOLORED);
			pt.DrawStringAt(20, 72, "violate the law)", &Font8, UNCOLORED);
		}
		tmpPrint = "Press \"ENTER\" to change";

		pt.DrawFilledRectangle(0, 174, 200, 190, COLORED);

		pt.DrawStringAt(10, 175, tmpPrint.c_str(), &Font12, UNCOLORED);
	}
	else
	{
		alertWindow("LoraSettings", "Error read settings");
	}
	pref.end();
	return 0;
}

int MenuDrawLoraSet()
{
	Preferences pref;
	int32_t print_rf_frequency;
	int32_t print_tx_output_power;
	int32_t print_lora_spreading_factor;
	int32_t print_lora_codingrate;
	int32_t print_lora_bandwidth;

	if (pref.begin("LoraSettings", false))
	{
		print_rf_frequency = pref.getInt("rf_frequency", LORAFREQ);
		print_tx_output_power = pref.getInt("tx_output_power", LORATXPOWER);
		print_lora_spreading_factor = pref.getInt("lora_spreading", LORASPREAD);
		print_lora_codingrate = pref.getInt("lora_codingrate", LORACODINGRATE);
		print_lora_bandwidth = loraBandtoNum(pref.getInt("lora_bandwidth", LORABAND), false);

		pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);

		float freqGet = print_rf_frequency / 10000;
		String tmpPrint = "Frequency: " + String(freqGet / 100) + " mHz";
		pt.DrawStringAt(10, 30, tmpPrint.c_str(), &Font12, COLORED);

		tmpPrint = "Power: " + String(print_tx_output_power) + " dBm";
		pt.DrawStringAt(10, 45, tmpPrint.c_str(), &Font12, COLORED);

		tmpPrint = "Bandwidth: " + String(print_lora_bandwidth) + " kHz";
		pt.DrawStringAt(10, 60, tmpPrint.c_str(), &Font12, COLORED);

		tmpPrint = "SF: " + String(print_lora_spreading_factor);
		pt.DrawStringAt(10, 75, tmpPrint.c_str(), &Font12, COLORED);

		if (print_lora_codingrate == 1)
		{
			tmpPrint = "C/R: 4/5"; // 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
		}
		else
		{
			tmpPrint = "C/R: " + String(print_lora_codingrate);
		}
		pt.DrawStringAt(10, 90, tmpPrint.c_str(), &Font12, COLORED);

		tmpPrint = "Preamble Lenght: " + String(LORA_PREAMBLE_LENGTH);
		pt.DrawStringAt(10, 105, tmpPrint.c_str(), &Font12, COLORED);

		tmpPrint = "Press \"ENTER\" to change";
		pt.DrawFilledRectangle(0, 174, 200, 190, COLORED);
		pt.DrawStringAt(10, 175, tmpPrint.c_str(), &Font12, UNCOLORED);
	}
	else
	{
		alertWindow("LoraSettings", "Error read settings");
	}

	pref.end();

	return 0;
}

uint8_t lastNumElements = 0;
bool CounterUsersUpdater()
{
	if (lastNumElements != numElements)
	{
		String allnum = "On:" + String(numElements);
		pt.DrawFilledRectangle(0, 0, 40, 10, COLORED);
		pt.DrawStringAt(2, 0, allnum.c_str(), &Font12, UNCOLORED);
		lastNumElements = numElements;
		return true;
	}
	else
	{
		return false;
	}
}

String getBattery()
{
	int adcValue = analogRead(PinBattery);
	String batterystate;
	int mapPercent = map(adcValue, 2827, 4095, 0, 100);
	if (mapPercent > 100 || mapPercent < 0)
	{
		batterystate = "Error";
	}
	else
	{
		batterystate = String(map(adcValue, 2827, 4095, 0, 100)) + "%";
	}

	ESP_LOGD("SYSTEM", "Battery %f", batterystate);

	return batterystate;
}

String getVoltage()
{
	int adcValue = analogRead(PinBattery); // 4095-2016
	float voltage = (adcValue * 4.2) / 4095;

	return String(voltage);
}

int MenuHeader(String middle_text)
{
	pt.DrawFilledRectangle(0, 0, 200, 11, COLORED);

	String allnum = "On:" + String(numElements);
	pt.DrawFilledRectangle(0, 0, 40, 10, COLORED);
	pt.DrawStringAt(2, 0, allnum.c_str(), &Font12, UNCOLORED);

	pt.DrawStringAt(100 - ((middle_text.length() * 7) / 2), 0, middle_text.c_str(), &Font12, UNCOLORED);
	String voltValue = getBattery();
	pt.DrawStringAt(200 - 7 * voltValue.length(), 0, voltValue.c_str(), &Font12, UNCOLORED);
	drawBatteryState(atoi(voltValue.c_str()));
	return 0;
}

int MenuDrawStatDb()
{
	pt.DrawFilledRectangle(0, 0, 200, 18, UNCOLORED);
	// pt.DrawStringAt(1, 4, "20:10", &Font12, COLORED);
	pt.DrawHorizontalLine(0, 20, 200, COLORED);

	return 0;
}

int MenuAllUserDraw(uint8_t frame = 0, uint8_t index = 0)
{
	
	uint8_t xcoor = 20;
	pt.DrawFilledRectangle(0, 23, 200, 200, UNCOLORED);

	if (numElements == 0)
	{
		alertWindow("No user. Press Menu", "Try again 30 sec.");
	}
	else
	{
		ESP_LOGD("USERS", "frame %d  - index %d", frame, index);
		char line[20];

		uint8_t maxprimntuser = 1;

		if (numElements > 10)
		{
			maxprimntuser = (frame * 10) + 10;
		}
		else
		{
			maxprimntuser = numElements;
		}

		for (int idx = frame * 10; idx < maxprimntuser; idx++)
		{
			if (firstHop[idx] == 0)
			{
				sprintf(line, "%d: %02X%02X%02X%02X (%d/%d)", idx + 1, (uint8_t)(nodeId[idx] >> 24), (uint8_t)(nodeId[idx] >> 16), (uint8_t)(nodeId[idx] >> 8), (uint8_t)(nodeId[idx] >> 0), db_getCountInputMessages(nodeId[idx], true), db_getCountInputMessages(nodeId[idx], false));
			}
			else
			{
				sprintf(line, "%d: %02X%02X%02X%02X* (%d/%d)", idx + 1, (uint8_t)(nodeId[idx] >> 24), (uint8_t)(nodeId[idx] >> 16), (uint8_t)(nodeId[idx] >> 8), (uint8_t)(nodeId[idx] >> 0), db_getCountInputMessages(nodeId[idx], true), db_getCountInputMessages(nodeId[idx], false));
			}
			pt.DrawStringAt(xcoor, 25 + (idx * 15), line, &Font12, COLORED);
			if (numElements > 10 && (idx == 10 || idx == 20 || idx == 40))
			{
				xcoor += 25;
			}
		}
	}
	return 0;
}

int db_getCountInputMessages(uint32_t userid, bool Destination)
{
	uint16_t counter = 0;
	for (uint16_t g = 0; g <= countMessage; g++)
	{
		if (Destination)
		{
			if (IndChat[g].OnLora_fromID == userid)
			{
				counter += 1;
			}
		}
		else
		{

			if (IndChat[g].OnLora_toID == userid)
			{
				counter += 1;
			}
		}
	}
	if (counter == 0)
	{
		return 0;
	}
	else
	{
		return counter;
	}
}

int chatSingleDraw(unsigned int activeUserId, uint8_t Upd = 0)
{

	pt.DrawFilledRectangle(0, 11, 200, 200, UNCOLORED);
	uint8_t ycoordinates = 165;

	String msgPrint, who;

	if (numElements == 0)
	{
		alertWindow("No user. Press Menu", "Try again 30 sec.");
	}
	else
	{
		String inkStr[countinskStr];
		for (int g = 0; g < countinskStr; g++)
		{
			inkStr[g] = "";
		}
		for (int t = 0; t < countMessage; t++)
		{
			if (IndChat[t].OnLora_fromID == activeUserId || IndChat[t].OnLora_toID == activeUserId)
			{

				if (IndChat[t].OnLora_toID == activeUserId)
				{
					who = "i:";
				}
				else
				{
					who = "mf:"; // mesh friend :)
				}

				if (IndChat[t].confirm)
				{
					who += ">";
				}

				msgPrint = who + IndChat[t].message;
				uint16_t lenmsg = msgPrint.length();
				uint8_t howStr = (int)(lenmsg / 27) + 1;

				allstrCounter += howStr;
				if (allstrCounter >= countinskStr)
				{
					allstrCounter = countinskStr;
				}

			
				if (howStr > 1)
				{
					uint16_t lastpos = 28;
					uint16_t startpos = 0;
					String line1;
					
					for (uint16_t c = 1; c <= howStr; c++)
					{
						line1 = msgPrint.substring(startpos, lastpos);
						startpos = lastpos;
						if (lastpos >= lenmsg)
						{
							lastpos = lenmsg;
						}
						else
						{
							lastpos += 28;
						}

						for (int g = 0; g < countinskStr - 1; g++)
						{
							inkStr[g] = inkStr[g + 1];
						}
						inkStr[countinskStr - 1] = line1;
					}
				}
				else
				{

					for (int g = 0; g < countinskStr - 1; g++)
					{
						inkStr[g] = inkStr[g + 1];
					}

					inkStr[countinskStr - 1] = msgPrint;
				}
			}
		}

		for (int h = 0; h <= 10; h++)
		{ 
			int ypos = ycoordinates - (h * 15);
			int howmes = countinskStr - 1 - h;
			if (howmes > 0)
			{
				pt.DrawStringAt(0, ypos, inkStr[howmes - Upd].c_str(), &Font12, COLORED);
			}
		}

		//	delete[] allTextArray;

	}

	pt.DrawHorizontalLine(2, 180, 196, COLORED);
	pt.DrawHorizontalLine(2, 198, 196, COLORED);

	pt.DrawVerticalLine(2, 180, 18, COLORED);
	pt.DrawVerticalLine(198, 180, 18, COLORED);

	pt.DrawStringAt(2, 183, "|", &Font12, COLORED);

	return 0;
}

int chatGenDraw(uint8_t Upd = 0)
{
	pt.DrawFilledRectangle(0, 11, 200, 180, UNCOLORED);
	uint8_t ycoordinates = 165;

	String msgPrint, who;

	if (numElements == 0)
	{
		alertWindow("No user. Press Menu", "Try again 30 sec.");
	}
	else
	{
		String inkStr[countinskStr];
		for (int g = 0; g < countinskStr; g++)
		{
			inkStr[g] = "";
		}

		for (int t = 0; t < countMessage; t++)
		{
			if (GenChat[t].OnLora_fromID != 0)
			{
				if (GenChat[t].OnLora_fromID == 1)
				{
					who = "im:";
				}
				else
				{
					char outputString[9];
					itoa(GenChat[t].OnLora_fromID, outputString, 16);
					who = String(outputString) + ":";
				}

				msgPrint = who + GenChat[t].message;

				uint16_t lenmsg = msgPrint.length();
				uint8_t howStr = (int)(lenmsg / 27) + 1;

				allstrCounter += howStr;
				if (allstrCounter >= countinskStr)
				{
					allstrCounter = countinskStr;
				}

				if (howStr > 1)
				{

					uint16_t lastpos = 28;
					uint16_t startpos = 0;
					String line1;
			
					for (uint16_t c = 1; c <= howStr; c++)
					{
						line1 = msgPrint.substring(startpos, lastpos);

						startpos = lastpos;
						if (lastpos >= lenmsg)
						{
							lastpos = lenmsg;
						}
						else
						{
							lastpos += 28;
						}

						for (int g = 0; g < countinskStr - 1; g++)
						{
							inkStr[g] = inkStr[g + 1];
						}
						inkStr[countinskStr - 1] = line1;
					}
				}
				else
				{

					for (int g = 0; g < countinskStr - 1; g++)
					{
						inkStr[g] = inkStr[g + 1];
					}

					inkStr[countinskStr - 1] = msgPrint;
				}
			}
		}

		for (int h = 0; h <= 10; h++)
		{
			int ypos = ycoordinates - (h * 15);
			int howmes = countinskStr - 1 - h;
			if (howmes > 0)
			{
				pt.DrawStringAt(0, ypos, inkStr[howmes - Upd].c_str(), &Font12, COLORED);
			}
		}
		//	delete[] allTextArray;
	}

	pt.DrawHorizontalLine(2, 180, 196, COLORED);
	pt.DrawHorizontalLine(2, 198, 196, COLORED);

	pt.DrawVerticalLine(2, 180, 18, COLORED);
	pt.DrawVerticalLine(198, 180, 18, COLORED);

	pt.DrawStringAt(2, 183, "|", &Font12, COLORED);

	return 0;
}

int tagSingleDraw(String activeTags, uint8_t Upd = 0)
{
	pt.DrawFilledRectangle(0, 11, 200, 200, UNCOLORED);
	uint8_t ycoordinates = 175;

	String msgPrint, who;


	String inkStr[countinskStr];
	for (int g = 0; g < countinskStr; g++)
	{
		inkStr[g] = "";
	}
	for (int t = 0; t < countMessage; t++)
	{
		String genchatMess = GenChat[t].message;

		if (genchatMess.indexOf(activeTags, 0) != -1)
		{

			uint16_t lenmsg = genchatMess.length();
			uint8_t howStr = (int)(lenmsg / 27) + 1; 

			allstrCounter += howStr; 
			if (allstrCounter >= countinskStr)
			{
				allstrCounter = countinskStr;
			}

			if (howStr > 1)
			{

				uint16_t lastpos = 28;
				uint16_t startpos = 0;
				String line1;
				for (uint16_t c = 1; c <= howStr; c++)
				{
					line1 = genchatMess.substring(startpos, lastpos);

					startpos = lastpos;
					if (lastpos >= lenmsg)
					{
						lastpos = lenmsg;
					}
					else
					{
						lastpos += 28;
					}

					for (int g = 0; g < countinskStr - 1; g++)
					{
						inkStr[g] = inkStr[g + 1];
					}
					inkStr[countinskStr - 1] = line1;
				}
			}
			else
			{
				for (int g = 0; g < countinskStr - 1; g++)
				{
					inkStr[g] = inkStr[g + 1];
				}

				inkStr[countinskStr - 1] = msgPrint;
			}
		}

		for (int h = 0; h <= 10; h++)
		{ 
			int ypos = ycoordinates - (h * 15);
			int howmes = countinskStr - 1 - h;
			if (howmes > 0)
			{
				pt.DrawStringAt(0, ypos, inkStr[howmes - Upd].c_str(), &Font12, COLORED);
			}
		}

		//	delete[] allTextArray;
	}
	return 0;
}

int drawUpdate()
{

	// int drawTime = millis();
	epd154bw.SetFrameMemory(pt.GetImage(), 0, 0, pt.GetWidth(), pt.GetHeight());
	epd154bw.HalLcd_Partial_Update();
	// ESP_LOGD("DRAW: ", "%d", millis() - drawTime);
	return 0;
}

int chatDrawOutmess()
{
	pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
	String resMess = outMessage + "|";
	uint16_t finish = outMessage.length();

	if (finish > 27)
	{
		counterResMess++;
		resMess = resMess.substring(counterResMess, finish);
		pt.DrawStringAt(3, 184, resMess.c_str(), &Font12, COLORED);
	}
	else
	{
		pt.DrawStringAt(3, 184, resMess.c_str(), &Font12, COLORED);
	}

	return 0;
}

int PrintFreeHeap()
{
	ESP_LOGD("Heap size:", "%d", ESP.getHeapSize());
	ESP_LOGD("Free Heap:", "%d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	ESP_LOGD("Min Free Heap:", "%d", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
	ESP_LOGD("Max Alloc Heap:", "%d", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

	return 0;
}

void setup()
{
	pref.begin("Update", false);
	stateUpdate = pref.getBool("state", false);
	pref.end();

	if (stateUpdate == false)
	{
		if (pref.begin("AllSettings", false))
		{
			uint8_t cpuFreq = pref.getInt("cpu_freq", 80);
			setCpuFrequencyMhz(cpuFreq); // set cpu frequency from settings
			if (cpuFreq > 60)
			{
				ledIndicate = true;
				LedStart(ledIndicate);
			}
			pref.end();
		}
	}
	else
	{
		setCpuFrequencyMhz(240);
	}

	ESP_LOGD("Version", "%s", FIRMVERS);

	// WiFi.disconnect(true);
	// WiFi.mode(WIFI_OFF);		
	// WiFi.disconnect();

	LedSystemStart();

	uint32_t Freq = getCpuFrequencyMhz();
	ESP_LOGD("CPU_FREQ:", "%d", Freq);
	Freq = getXtalFrequencyMhz();
	ESP_LOGD("XTAL_FREQ:", "%d", Freq);
	Freq = getApbFrequency();
	ESP_LOGD("APB_FREQ:", "%d", Freq);

	epd154bw.EPD_Init();
	vTaskDelay(200 / portTICK_PERIOD_MS);
	epd154bw.DisplayPartBaseImage(gImage_backg);
	vTaskDelay(200 / portTICK_PERIOD_MS);
	epd154bw.HalLcd_Partial();
	vTaskDelay(200 / portTICK_PERIOD_MS);

	pt.SetWidth(200);
	pt.SetHeight(200);
	pt.Clear(UNCOLORED);
	pt.SetRotate(ROTATE_270);

	vTaskDelay(200 / portTICK_PERIOD_MS);
	if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire))
	{
		ESP_LOGD("Keypad", "keypad not found, check wiring & pullups!");
		alertWindow("Keypad", "keypad not found, check wiring & pullups!");
	}
	else
	{
		ESP_LOGD("KeyPad", "Start");
		keypad.matrix(5, 8);
		pinMode(PinIRQKeypad, INPUT);
		attachInterrupt(digitalPinToInterrupt(PinIRQKeypad), TCA8418_irq, CHANGE);
		keypad.flush();
		keypad.enableInterrupts();
	}
	MenuHeader("LoraType");

	uint8_t deviceMac[8];
	BoardGetUniqueId(deviceMac);
	deviceID += (uint32_t)deviceMac[2] << 24;
	deviceID += (uint32_t)deviceMac[3] << 16;
	deviceID += (uint32_t)deviceMac[4] << 8;
	deviceID += (uint32_t)deviceMac[5];
	char DeviceIDmac[9];
	itoa(deviceID, DeviceIDmac, 16);


	if (stateUpdate)
	{
		pref.begin("Update");
		pref.putBool("state", false);
		pref.end();
		OtaUpdate();
	}

	pt.DrawStringAt(30, 45, "LoraType..|", &Font20, COLORED);

	String output = "";

	output = "Cpu Frequency: " + String(getCpuFrequencyMhz()) + "mHz";
	pt.DrawStringAt(10, 95, output.c_str(), &Font12, COLORED);

	output = "Firmware Ver.:" + String(FIRMVERS);
	pt.DrawStringAt(10, 110, output.c_str(), &Font12, COLORED);

	output = "Your NodeId: " + String(DeviceIDmac);
	pt.DrawStringAt(10, 125, output.c_str(), &Font12, COLORED);

	if (!initLoRa())
	{
		pt.DrawStringAt(10, 140, "LoRa init - Error", &Font12, COLORED);
	}
	else
	{
		pt.DrawStringAt(10, 140, "LoRa init - Ok", &Font12, COLORED);
	}

	String batteryVal = getBattery();
	if (batteryVal == "Error")
	{
		output = "Battery - Error";
	}
	else
	{
		output = "Battery charge: " + batteryVal + "/" + getVoltage() + " v";
	}
	pt.DrawStringAt(10, 155, output.c_str(), &Font12, COLORED);

	if (pref.begin("LoraSettings", false))
	{
		float freq = pref.getInt("rf_frequency", LORAFREQ) / 10000;
		ESP_LOGD("LORA", "%f", freq);
		output = "Lora Frequency: " + String(freq / 100) + "mHz";
		pref.end();
	}
	else
	{
		ESP_LOGD("PREFERENCES", "FAIL get lora settings");
	}
	pt.DrawStringAt(10, 170, output.c_str(), &Font12, COLORED);

	pt.DrawFilledRectangle(0, 184, 200, 200, COLORED);
	pt.DrawStringAt(10, 185, "Let's start.. Press MENU", &Font12, UNCOLORED);

	drawUpdate();
	// delay(100);
	sendRandom = millis();
	itwork = sendRandom;
}

void loop()
{
	if (OnLoraFlag)
	{
		OnLoraFlag = false;
		ESP_LOGD("LORA_Payload_len", "%d", OnLora_rxPayload.length());
		ESP_LOGD("LORA_Payload", "%s", OnLora_rxPayload);
		if (OnLora_rxPayload[0] == '@')
		{
			ESP_LOGD("LORA", "Payload @");
			OnLora_rxPayload.remove(0, 1);
			for (int i = 0; i < countMessage; i++)
			{
				ESP_LOGD("LORA", "Confirm %s - Payload %s", IndChat[i].confirmCode, OnLora_rxPayload);
				if (OnLora_rxPayload == IndChat[i].confirmCode)
				{
					IndChat[i].confirm = true;
					ESP_LOGD("LORA", "%d - Confirm be added", i);
					break;
				}
			}
		}
		else if (OnLora_rxPayload[0] == '~')
		{
			db_addIndMessage(OnLora_rxfromID, 13, OnLora_rxPayload, OnLora_rxSize, OnLora_rxRssi, OnLora_rxSnr);
			// vTaskDelay(500 / portTICK_PERIOD_MS);
			loraSendConfirm(OnLora_rxfromID, getConfirmCode(OnLora_rxPayload));
			LedLoraInputMess(true);
		}
		else
		{
#ifdef selfBroadMessaging
			// This adds signal quality data to the messages
			byte onloraRssi = map(OnLora_rxRssi, -136, -30, 0, 100);
			OnLora_rxPayload = OnLora_rxPayload + "-" + String(onloraRssi) + "%";
#endif
			db_addGenMessage(OnLora_rxfromID, OnLora_rxPayload, OnLora_rxSize, OnLora_rxRssi, OnLora_rxSnr);
			LedLoraInputMess(false);
			ESP_LOGD("LORA", "Add broad. mess");
			// db_printGenALL();
		}

		if (isGenChat)
		{
			chatGenDraw();
			drawUpdate();
		}
		if (isIndChat)
		{
			chatSingleDraw(getActiveUser);
			drawUpdate();
		}
	}

	if (TCA8418_event == true)
	{
		int keyNow = keypad.getEvent();
		keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
		int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);
		if ((intstat & 0x01) == 0)
		{
			TCA8418_event = false;
		}

		bool PRFlag = false;
		if (keyNow & 0x80)
		{
			// ESP_LOGD("KEYPAD", "Press");
			PRFlag = true;
			keyNow &= 0x7F;
			keyNow--;
			byte col = keyNow / 10;
			byte row = keyNow % 10;
			ESP_LOGD("k", "%d", keyNow);
			ESP_LOGD("MenuNow", "%d", MenuNow());
			if (keyNow == keyServ::keyCTRL)
			{
				isCtrlActive = true;
				ESP_LOGD("keypad", "CTRL IS TRUE");
			}
			else if (keyNow == keyServ::keyMENU) // Main Menu
			{
				MenuHistory(-1);
				isIndChat = false;
				isGenChat = false;
				outMessage = "";
				MenuHeader("Menu");
				MenuDraw();
				MenuDrawArow(menuNowSelect);
				drawUpdate();
			}
			if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::EVERYTHING) // Enter
			{
				MenuHistory(menuList::EVERYTHING);
				MenuHeader(String(menuAll[menuNowSelect].itemName));
				isIndChat = false;
				isGenChat = true;
				chatGenDraw();
				drawUpdate();
			}
			else if (MenuNow() == menuList::EVERYTHING) // We are in the main chat
			{
				if (symCheck(keyNow) == 0)
				{
					if (isCtrlActive)
					{
						outMessage += keySymbCase[col][row];
					}
					else
					{
						outMessage += keyDownCase[col][row];
					}

					ESP_LOGD("DRAW", "draw in everything");
					chatDrawOutmess();
					drawUpdate();
				}
				else if (keyNow == keyServ::keyENTER)
				{
					ESP_LOGD("LORA", "Send message in everything chat");
					if (outMessage.length() >= minLenghtOutMessage)
					{
						if (!loraSendBroadcast(outMessage))
						{
							outMessage = "";
							LedLoraWarning(true);
							pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
							chatGenDraw();
						}
						else
						{
							LedLoraWarning(false);
						}
						drawUpdate();
					}
					else
					{
						LedSystemWarning();
					}
				}
				else if (keyNow == keyServ::keyBACKSPACE)
				{
					outMessage.remove(outMessage.length() - 1, 1);
					chatDrawOutmess();
					drawUpdate();
				}
				else if (keyNow == keyServ::keyUP)
				{
					if (messFrame < countinskStr && (allstrCounter - 10) > 1)
					{
						messFrame++;
						chatGenDraw(messFrame);
						drawUpdate();
					}
				}
				else if (keyNow == keyServ::keyDOWN)
				{
					if (messFrame > 0)
					{
						messFrame--;
						chatGenDraw(messFrame);
						drawUpdate();
					}
				}
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::USERS) // Enter to Users online list
			{
				MenuHistory(menuList::USERS);
				MenuHeader(String(menuAll[menuNowSelect].itemName));
				MenuAllUserDraw();
				if (numElements > 0)
				{
					MenuDrawArow(0);
				}
				else
				{
					alertWindow("No users found", "Refresh in 30 seconds");
				}
				drawUpdate();
			}
			else if (MenuNow() == menuList::USERS)
			{
				static uint8_t userindex = 0, userFrame = 0, stage = 0;

				ESP_LOGD("KEYPAD_UP", "User frame: %d  |  User Index:  %d  | Stage %d  |  NumElement %d", userFrame, userindex, stage, numElements);

				if (stage == 0)
				{
					ESP_LOGD("CHAT IND", " Stage 0");
					if (keyNow == keyServ::keyUP && userindex > 0)
					{
						if (userindex == 0 && (numElements % 10 >= 2))
						{
							userFrame -= 1;
							userindex = 9;
						}
						userindex -= 1;

						MenuAllUserDraw(userFrame, userindex);
						MenuDrawArow(userindex);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyDOWN && userindex <= 10 && userindex < numElements - 1)
					{
						if (userindex == 10 && (numElements % 10 >= 2))
						{
							userFrame += 1;
							userindex = 0;
						}
						userindex += 1;

						ESP_LOGD("KEYPAD_DOWN", "%d - %d", userFrame, userindex);
						MenuAllUserDraw(userFrame, userindex);
						MenuDrawArow(userindex);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyOKCenter)
					{
						// start individual chat here
						stage = 1;
						isGenChat = false;
						isIndChat = true;
						getActiveUser = nodeId[userindex];
						MenuHeader(String(getActiveUser, HEX));
						chatSingleDraw(getActiveUser);
						drawUpdate();
					}
				}
				else if (stage == 1)
				{
					ESP_LOGD("CHAT IND", " Stage 1");
					/*==========================================================*/
					if (symCheck(keyNow) == 0)
					{
						if (isCtrlActive)
						{
							outMessage += keySymbCase[col][row];
						}
						else
						{
							outMessage += keyDownCase[col][row];
						}

						ESP_LOGD("DRAW", "draw in individual chat");
						chatDrawOutmess();
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						ESP_LOGD("LORA", "Send message in individual chat");
						if (outMessage.length() >= minLenghtOutMessage)
						{
							loraSendMessage(getActiveUser, outMessage);
							outMessage = "";
							counterResMess = 0;
							pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
							chatSingleDraw(getActiveUser);
							drawUpdate();
						}
						else
						{
							LedSystemWarning();
						}
					}
					else if (keyNow == keyServ::keyBACKSPACE)
					{
						outMessage.remove(outMessage.length() - 1, 1);
						chatDrawOutmess();
						drawUpdate();
					}
					else if (keyNow == keyServ::keyUP)
					{
						if (messFrame < countinskStr && (allstrCounter - 10) > 1)
						{
							messFrame++;
							chatSingleDraw(getActiveUser, messFrame);
							drawUpdate();
						}
					}
					else if (keyNow == keyServ::keyDOWN)
					{
						if (messFrame > 0)
						{
							messFrame--;
							chatSingleDraw(getActiveUser, messFrame);
							drawUpdate();
						}
					}
					else if (keyNow == keyServ::keyLEFT)
					{
						ESP_LOGD("KEYPAD", "LEFT-ok");
						stage = 0;
						isIndChat = false;
						MenuHistory(menuList::USERS);
						MenuHeader(String(menuAll[menuNowSelect].itemName));
						MenuAllUserDraw();
						if (numElements > 0)
						{
							MenuDrawArow(0);
						}
						else
						{
							alertWindow("No users found", "Refresh in 30 seconds");
						}
						drawUpdate();
					}

					/*==========================================================*/
				}
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::FQA)
			{

				MenuHistory(menuList::FQA);
				MenuHeader(String(menuAll[menuNowSelect].itemName));
				MenuDrawDistance();
				drawUpdate();
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyUP) // UP
			{
				if (menuNowSelect <= menuCount - 1 && menuNowSelect > 0)
				{
					ESP_LOGD("KEYPAD", "UP");
					menuNowSelect = menuNowSelect - 1;
					ESP_LOGD("MENU", "%d", menuNowSelect);
					MenuDrawArow(menuNowSelect);
					drawUpdate();
				}
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyDOWN) // DOWN
			{
				if (menuNowSelect < menuCount - 1 && menuNowSelect >= 0)
				{
					ESP_LOGD("KEYPAD", "DOWN");
					menuNowSelect = menuNowSelect + 1;
					ESP_LOGD("MENU", "%d", menuNowSelect);
					MenuDrawArow(menuNowSelect);
					drawUpdate();
				}
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::LORASETTINGS)
			{
				MenuHistory(menuList::LORASETTINGS);
				MenuHeader(String(menuAll[menuList::LORASETTINGS].itemName));
				MenuDrawLoraSet();
				drawUpdate();
			}
			else if (MenuNow() == menuList::LORASETTINGS)
			{
				static String freq; // Set frequency
				static uint8_t stage = 0;
				static uint8_t setModeLora = 0;
				// https://development.libelium.com/lora_networking_guide/transmission-modes
				//  mode / BW /  CR /   SF  / Sensitivity (dB) / Transmission time
				int16_t modeLora[7][6] = {
					{1, 125, 1, 12, -134, 5781}, // max range, slow data rate
					{2, 250, 1, 12, -131, 3287},
					{3, 500, 1, 12, -128, 2040},
					{4, 250, 1, 10, -126, 1457},
					{5, 250, 1, 9, -123, 1145},
					{6, 500, 1, 8, -117, 890},
					{7, 500, 1, 7, -114, 848}}; // min range, fast data rate, minimum battery impact

				uint8_t powerLora[9] = {14, 15, 16, 17, 18, 19, 20, 21, 22};
				static uint8_t counterPower = 0;
				String tempTextSet;

				if (keyNow == keyServ::keyENTER && stage == 0)
				{
					stage = 1; // It's a trick)))
					drawSetFreq(freq);
					drawUpdate();
				}
				else if (stage == 1)
				{
					ESP_LOGD("KEYPAD", "Stage 1 settings");
					if (symCheck(keyNow) == 0)
					{
						char symb = keySymbCase[col][row];
						ESP_LOGD("keypad SYMBOL", "%s", String(symb));
						int num = atoi(&symb);
						if (num >= 0 && num <= 9)
						{
							freq += String(num);
							drawSetFreq(freq);
							drawUpdate();
						}
					}
					else if (keyNow == keyServ::keyBACKSPACE)
					{
						freq.remove(freq.length() - 1, 1);
						drawSetFreq(freq);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						stage = 2;

						tempTextSet = "Power: <- 14 -> dBm";
						pt.DrawFilledRectangle(10, 60, 200, 73, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						pt.DrawStringAt(10, 75, "- (14-22dBm. >14dBm may be illegal)", &Font8, COLORED);
						drawUpdate();
					}
				}
				else if (stage == 2)
				{
					ESP_LOGD("STAGE SETTINGS", "Stage 2 ");
					if (keyNow == keyServ::keyRIGHT && counterPower <= 8 && counterPower >= 0)
					{
						counterPower += 1;
						tempTextSet = "Power: <- " + String(powerLora[counterPower]) + " -> dBm";
						pt.DrawFilledRectangle(10, 60, 200, 75, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyLEFT && counterPower >= 0 && counterPower <= 8)
					{
						counterPower -= 1;
						tempTextSet = "Power: <- " + String(powerLora[counterPower]) + " -> dBm";
						pt.DrawFilledRectangle(10, 60, 200, 75, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						stage = 3;

						drawSetOpMode(setModeLora, modeLora);
						drawUpdate();
					}
				}
				else if (stage == 3)
				{
					if (keyNow == keyServ::keyRIGHT && setModeLora < 6 && setModeLora >= 0)
					{
						setModeLora += 1;
						drawSetOpMode(setModeLora, modeLora);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyLEFT && setModeLora > 0 && setModeLora <= 6)
					{
						setModeLora -= 1;
						drawSetOpMode(setModeLora, modeLora);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						Preferences pref;
						if (pref.begin("LoraSettings", false))
						{
							int freq_set = freq.toInt() * 100000;
							if (freq_set < 82000 || freq_set > 92000)
							{
								alertWindow("Error Freq range", "Pres Menu");
								vTaskDelay(5000 / portTICK_PERIOD_MS);
								ESP.restart();
							}
							else
							{
								pref.putInt("rf_frequency", freq_set);
							}
							ESP_LOGD("SETTINGS", "%d", freq_set);
							pref.putInt("tx_output_power", powerLora[counterPower]);
							pref.putInt("lora_spreading", modeLora[setModeLora][3]);
							pref.putInt("lora_codingrate", modeLora[setModeLora][2]);

							pref.putInt("lora_bandwidth", loraBandtoNum(modeLora[setModeLora][1]));
							pref.end();
							alertWindow("Data saved", "Reboot in progress");
							drawUpdate();

							vTaskDelay(5000 / portTICK_PERIOD_MS);
							ESP.restart();
						}
						else
						{
							alertWindow("Error saving settings", "Press Menu and try again");
							drawUpdate();
						}
					}
				}
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::ALLSETTINGS)
			{
				MenuHistory(menuList::ALLSETTINGS);
				MenuHeader(String(menuAll[menuList::ALLSETTINGS].itemName));
				MenuDrawAllSet();
				drawUpdate();
			}
			else if (MenuNow() == menuList::ALLSETTINGS)
			{
				String tempTextSet;
				static uint8_t stage = 0;
				int cpufreqs[6] = {240, 160, 80, 40, 20, 10};
				static bool legality = true, reset = false;
				static uint8_t counterFreq = 0;

				if (keyNow == keyServ::keyENTER && stage == 0)
				{
					stage = 1;
					tempTextSet = "CPU Frequency: <- " + String(cpufreqs[counterFreq]) + " -> MHz";
					pt.DrawFilledRectangle(0, 30, 200, 200, UNCOLORED);
					pt.DrawStringAt(10, 30, tempTextSet.c_str(), &Font12, COLORED);
					drawUpdate();
				}
				else if (stage == 1)
				{
					if (keyNow == keyServ::keyRIGHT && counterFreq < 5 && counterFreq >= 0)
					{
						counterFreq += 1;
						tempTextSet = "CPU Frequency: <- " + String(cpufreqs[counterFreq]) + " -> MHz";

						pt.DrawFilledRectangle(0, 30, 200, 45, UNCOLORED);
						pt.DrawStringAt(10, 30, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyLEFT && counterFreq > 0 && counterFreq <= 5)
					{
						counterFreq -= 1;
						tempTextSet = "CPU Frequency: <- " + String(cpufreqs[counterFreq]) + " -> MHz";
						pt.DrawFilledRectangle(10, 30, 200, 45, UNCOLORED);
						//	pt.DrawFilledRectangle(4, 30, 8, 75, COLORED);
						pt.DrawStringAt(10, 30, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						stage = 2;
						tempTextSet = "Legality: <- Yes ->";
						pt.DrawFilledRectangle(10, 45, 200, 60, UNCOLORED);
						pt.DrawStringAt(10, 45, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
				}
				else if (stage == 2)
				{
					if (keyNow == keyServ::keyRIGHT)
					{
						legality = false;
						tempTextSet = "Legality: <- No ->";
						pt.DrawFilledRectangle(0, 45, 200, 60, UNCOLORED);
						pt.DrawStringAt(10, 45, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyLEFT)
					{
						legality = true;
						tempTextSet = "Legality: <- Yes ->";
						pt.DrawFilledRectangle(10, 45, 200, 60, UNCOLORED);
						pt.DrawStringAt(10, 45, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						stage = 3;
						ESP_LOGD("SETTINGS", "%d", stage);
						tempTextSet = "Reset settings: <- No ->";
						pt.DrawFilledRectangle(10, 60, 200, 75, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
				}
				else if (stage == 3)
				{

					if (keyNow == keyServ::keyRIGHT)
					{
						reset = false;
						ESP_LOGD("reset setting", "false");
						tempTextSet = "Reset settings: <- No ->";
						pt.DrawFilledRectangle(10, 60, 200, 75, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyLEFT)
					{
						reset = true;
						ESP_LOGD("reset setting", "true");
						tempTextSet = "Reset settings: <- Yes ->";
						pt.DrawFilledRectangle(10, 60, 200, 75, UNCOLORED);
						pt.DrawStringAt(10, 60, tempTextSet.c_str(), &Font12, COLORED);
						drawUpdate();
					}
					else if (keyNow == keyServ::keyENTER)
					{
						if (reset == true)
						{

							pref.begin("AllSettings", false);
							pref.clear();
							pref.end();
							pref.begin("LoraSettings");
							pref.clear();
							pref.end();
							alertWindow("All set. reset", "Reboot in progress");
							drawUpdate();
							vTaskDelay(5000 / portTICK_PERIOD_MS);
							ESP.restart();
						}
						else
						{
							if (pref.begin("AllSettings", false))
							{
								pref.putBool("legality", legality);
								pref.putInt("cpu_freq", cpufreqs[counterFreq]);
								alertWindow("Data saved", "Reboot in progress");
								drawUpdate();
								pref.end();
								vTaskDelay(5000 / portTICK_PERIOD_MS);
								ESP.restart();
							}
							else
							{
								alertWindow("Error saving settings", "Press Menu and try again");
							}
						}
					}
				}
			}
			else if (col == 3 && row == 3) // Left
			{
				// LEFT
				menuCount = 6;
				menuNowSelect = menuHis[4];
			}
			else if (col == 3 && row == 1) // Right
			{
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::ABOUT) // about bro
			{
				MenuHistory(menuList::ABOUT);
				MenuHeader(String(menuAll[menuNowSelect].itemName));
				drawAbout();
				drawUpdate();
			}
			else if (MenuNow() == menuList::MAINMENU && keyNow == keyServ::keyOKCenter && menuNowSelect == menuList::UPDATE)
			{
				MenuHeader(String(menuAll[menuNowSelect].itemName));
				pref.begin("Update", false);
				pref.putBool("state", true);
				pref.end();
				alertWindow("OTAUpdate", "Rebooting");
				ESP_LOGD("OTAUPDATE", "Rebooting and start OTA server");
				drawUpdate();
				vTaskDelay(2000);
				ESP.restart();
			}
			else
			{
				ESP_LOGD("KEYPAD", "%c", keyDownCase[keyNow / 10][keyNow % 10]);
			}
		}
		else
		{
			//	ESP_LOGD("KEYPAD", "Release");
			keyNow &= 0x7F;
			keyNow--;
			if (keyNow == keyServ::keyCTRL)
			{
				isCtrlActive = false;
				//	ESP_LOGD("KEYPAD", "CTRL Release FALSE");
			}
			PRFlag = false;
		}
	}

#ifdef selfBroadMessaging
	if (numElements > 0 && isGenChat && (millis() - sendBrodcast) >= (30000 + random(100, 5000)))
	{
		loraSendBroadcast(selfText[random(0, 16)]);
		chatGenDraw();
		drawUpdate();
		sendBrodcast = millis();
	}
#endif
	if ((millis() - sendRandom) >= 20000)
	{
		if (CounterUsersUpdater())
		{
			ESP_LOGD("LORA USERS", "Counter Update");
			drawUpdate();
		}
		sendRandom = millis();
	}

	if ((millis() - itwork) >= 10000)
	{
		heartLed();
		itwork = millis();
	}

	if (nodesListChanged)
	{
		//  Nodes list changed, update display and report it
		nodesListChanged = false;
		if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
		{
			numElements = numOfNodes();
			for (int idx = 0; idx < numElements; idx++)
			{
				getNode(idx, nodeId[idx], firstHop[idx], numHops[idx]);
			}
			xSemaphoreGive(accessNodeList);

			ESP_LOGD("LORA", "%d nodes in the map. My node #01 id: %08X ", numElements + 1, deviceID);
		}
		else
		{
			ESP_LOGD("LORA", "Could not access the nodes list");
		}
	}
}
