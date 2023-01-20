#include "main.h"
					
#define APIKEY "*****************************"
#define PINGADR "95.216.56.89"
#define FIRMVERS "2.0.1"

#define inkRST_PIN 32
#define inkDC_PIN 12
#define inkCS_PIN 15
#define inkBUSY_PIN 27
#define inkCLK_PIN 14

#define COLORED 0
#define UNCOLORED 1

#define DOWN char(25)
#define UP char(21)
#define LEFT char(23)
#define RIGHT char(24)
#define CTRL char(29)
#define SPACE char(28)
#define ENTER char(27)
#define OK char(22)
#define MENU char(20)
#define BACKSPACE char(26)

bool stateUpdate = false;
Preferences pref;

unsigned char img[5000], imgoutmess[1000];

Paint pt(img, 0, 0), ptChat(imgoutmess, 0, 0);
Tca8418Keyboard key_matrix(6, 6);
DEPG0150BxS810FxX_BW epd154bw(inkRST_PIN, inkDC_PIN, inkCS_PIN, inkBUSY_PIN, inkCLK_PIN);

dataMsg outData;
nodesList routeToNode;
uint32_t nodeId[48], firstHop[48];
uint8_t numHops[48], numElements;

uint8_t messFrame = 0;

boolean nodesListChanged = false;
time_t sendRandom, itwork, sendBrodcast;
bool workFlag = 1;

unsigned long getActiveUser = 0;
String getActiveTag = "";
int dbRes_argcount = 0;
uint16_t allstrCounter = 0;

char sym;
unsigned int sqlLastCount = 0;
String outMessage = "";

bool OnLoraFlag = false;

uint32_t OnLora_rxfromID;
String OnLora_rxPayload;
uint16_t OnLora_rxSize;
int16_t OnLora_rxRssi;
int8_t OnLora_rxSnr;

String selfText[21] = {
	"Asking someone to be quiet or to shut up.",
	"Starting over again on a new design from a previously failed attempt.",
	"A task that's easy to accomplish, a thing lacking complexity.",
	"What you would expect to happen; something normal or common",
	"Coming close to a successful outcome only to fall short at the end.",
	"Iron pyrities is a worthless mineral that resembles gold.",
	"Spending time with another to strengthen the relationship",
	"Often said by the winner in poker, as the others 'weep' over the loss.",
	"People tend to associate with others who share similar interests or values",
	"Someone that calls for help when it is not needed. Someone who is lying.",
	"To not work alone, but rather, together with others in order to achieve a certain goal.",
	"0Putting yourself in a risky situation in order to help someone; or to hazard a guess.",
	"Having confidence in a specific outcome; being almost sure about something.",
	"Anything that requires minimal brain activity to accomplish.",
	"Finishing a task without making any excuses.",
	" Someone or something that one finds to be agreeable or delightful.",
	"Something that occurs too early before preparations are ready. Starting too soon",
	"To cut the mustard is to meet a required standard, or to meet expectations.",
	"Recalling a memory; causing a person to remember something or someone.",
	"To make a wrong assumption about something.",
	"Often said by the winner in poker, as the others 'weep' over the loss."};

uint8_t y_dis, counterResMess = 0;

bool keyboardUpperFlag = false;
char arrSymbol[9][6] = {
	{'0', 'i', 'u', 'y', 't', 'r'},
	{'e', 'w', 'q', '0', '0', 'k'},
	{'j', 'h', 'g', 'f', 'd', 's'},
	{'a', '0', '0', 25, 'b', 'v'},
	{28, 'c', 'x', 'z', 'n', '0'},
	{'0', 22, 24, 'o', 23, 21},
	{'l', 'm', 29, '0', '0', '0'},
	{'0', '0', '0', 20, 26, 'p'},
	{27, '0', '0', '0', '0', '0'}};

char arrUpperSymbol[9][6] = {
	{'0', 'I', 'U', 'Y', 'T', 'R'},
	{'E', 'W', 'Q', '0', '0', 'K'},
	{'J', 'H', 'G', 'F', 'D', 'S'},
	{'A', '0', '0', 25, 'B', 'V'},
	{28, 'C', 'X', 'Z', 'N', '0'},
	{'0', 22, 24, 'O', 23, 21},
	{'L', 'M', 29, '0', '0', '0'},
	{'0', '0', '0', 20, 26, 'P'},
	{27, '0', '0', '0', '0', '0'}};

char arrCtrlSymbol[9][6] = {
	{'0', '-', '_', ')', '(', '3'},
	{'2', '1', '#', '0', '0', '"'},
	{';', ':', '/', '6', '5', '4'},
	{'*', '0', '0', '0', '!', '?'},
	{'0', '9', '8', '7', ',', '0'},
	{'0', '0', '0', 'o', '0', '0'},
	{'$', '.', '0', '0', '0', '0'},
	{'0', '0', '0', '0', '0', '@'},
	{'0', '0', '0', '0', '0', '0'}};

struct menuitem
{
	bool active;
	unsigned int itemNum;
	const char *itemName;
};

menuitem menuAll[] =
	{
		{false, 0, "Everything chat"},
		{false, 1, "Users Online"},
		{false, 2, "All #Tags"},
		{false, 3, "Lora Settings"},
		{false, 4, "Friend Quality"},
		{false, 5, "Update firmware"},
		{false, 6, "About"}};

const int countMessage = 30;
const int countTags = 300;
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
	int rssi;
	int snr;
};

distanceNode dnode[48];

int menuCount = 6;
char symHis[5], symLast = '~';
int menuHis[5], menuLast = 0;

bool isIndChat = false, isGenChat = false, isTagsView = false;
int menuNowSelect = 0;

#include "ledFunction.h"

void onUpdateProgress(int progress, int totalt);

int drawUpdate(); // Print to display
int drawAbout();
int MenuDraw();																						 // Draw all menu
bool CounterUsersUpdater();																			 // Print all users in mesh
int MenuAllUserDraw();																				 // Print arrow in all menu
int MenuDrawArow(int);																				 // Print single Chat 1:1
int chatSingleDraw(unsigned int, uint8_t);															 // Chat where is all users in mesh
int chatGenDraw(int);																				 // Send message to all
int loraSendBroadcast(String);																		 // Send message to user
int loraSendMessage(unsigned long, String);															 // Print lora settings
int MenuDrawLoraSet();																				 // Print free heap in device
int PrintFreeHeap();																				 // Print all data in RAM memory
int db_printALL();																					 // Callback message on LORA
void OnLoraData(uint32_t fromID, uint8_t *rxPayload, uint16_t rxSize, int16_t rxRssi, int8_t rxSnr); // Callback when Node list Changed every 30 sec
void onNodesListChange(void);																		 // Keyboard history
int symHistory(char);																				 // Add message to database
int db_addGenMessage(uint32_t OnLora_rxfromID, String OnLora_rxPayload, uint16_t OnLora_rxSize, int16_t OnLora_rxRssi, int8_t OnLora_rxSnr);
int db_addIndMessage(uint32_t dbfromID, uint32_t dbtoID, String dbrxPayload, uint16_t dbrxSize, int16_t dbxRssi, int8_t dbrxSnr); // Return the currently pressed button
char checkButton();																												  // Print all tags
int MenuAllTagsDraw();
int MenuHeader(String);
int LedUpperCase(bool);
String getConfirmCode(String);

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
	pt.DrawStringAt(20, 80, "https://Automation.art", &Font12, COLORED);

	pt.DrawStringAt(3, 95, "Meet LoraType - the urban 'teletype", &Font8, COLORED);
	pt.DrawStringAt(3, 105, "tweeting'. It uses radio signal on", &Font8, COLORED);
	pt.DrawStringAt(3, 115, "free frequency.It uses radio signal", &Font8, COLORED);
	pt.DrawStringAt(3, 125, "on free frequency,which allows users", &Font8, COLORED);
	pt.DrawStringAt(3, 135, "to be independent of Int.connection,", &Font8, COLORED);
	pt.DrawStringAt(3, 145, "which is crucial in case of any ", &Font8, COLORED);
	pt.DrawStringAt(3, 155, "armageddon situation.", &Font8, COLORED);
	pt.DrawStringAt(3, 165, "It works like walkie talkie.", &Font8, COLORED);

	drawUpdate();

	return 0;
}

void onUpdateProgress(int progress, int totalt)
{
	static int last = 0;
	static int x_cor = 0;
	int progressPercent = (100 * progress) / totalt;

	if (last != progressPercent && progressPercent % 10 == 0)
	{

		String progress = "." + String(progressPercent) + ".";
		x_cor += 15;
		pt.DrawStringAt(x_cor, y_dis, progress.c_str(), &Font8, COLORED);
		drawUpdate();
	}
	last = progressPercent;
}

int OtaUpdate()
{
	y_dis = 20;
	MenuHeader("LoraType Update");
	pt.DrawStringAt(10, y_dis, "Update firmware start", &Font8, COLORED);
	y_dis += 14;
	pt.DrawStringAt(10, y_dis, "Enter Wifi: ", &Font12, COLORED);
	String wifi_name = "", wifi_pass = "";
	y_dis += 14;
	pt.DrawStringAt(3, y_dis, "->", &Font12, COLORED);
	drawUpdate();

	while (1)
	{
		sym = checkButton();
		if (sym != '~')
		{
			symHistory(sym);
			if (sym == ENTER)
			{
				break;
			}
			else if (sym == CTRL)
			{
			}
			else if (sym == BACKSPACE)
			{
				wifi_name.remove(wifi_name.length() - 1, 1);
			}
			else
			{
				wifi_name += sym;
			}

			pt.DrawFilledRectangle(16, y_dis - 2, 200, y_dis + 14, UNCOLORED);
			pt.DrawStringAt(16, y_dis, wifi_name.c_str(), &Font12, COLORED);
			drawUpdate();

			sym = '~';
		}
	}

	y_dis += 14;
	pt.DrawStringAt(3, y_dis, "->", &Font12, COLORED);
	drawUpdate();

	while (1)
	{
		sym = checkButton();
		if (sym != '~')
		{
			symHistory(sym);
			if (sym == ENTER)
			{
				break;
			}
			else if (sym == CTRL)
			{
			}
			else if (sym == BACKSPACE)
			{
				wifi_pass.remove(wifi_pass.length() - 1, 1);
			}
			else
			{
				wifi_pass += sym;
			}

			pt.DrawFilledRectangle(16, y_dis - 2, 200, y_dis + 14, UNCOLORED);
			pt.DrawStringAt(16, y_dis, wifi_pass.c_str(), &Font12, COLORED);
			drawUpdate();
			sym = '~';
		}
	}

	WiFi.begin(wifi_name.c_str(), wifi_pass.c_str());

	String network = "Connect to WiFi.";
	y_dis += 16;

	while (WiFi.status() != WL_CONNECTED)
	{

		network += "..";
		pt.DrawFilledRectangle(16, y_dis - 2, 200, y_dis + 9, UNCOLORED);
		pt.DrawStringAt(10, y_dis, network.c_str(), &Font8, COLORED);
		drawUpdate();
		delay(1000);
	}

	if (WiFi.status() == WL_CONNECTED)
	{
		bool success = Ping.ping(PINGADR, 3);
		if (success)
		{

			y_dis += 10;
			pt.DrawStringAt(10, y_dis, "OTA Server available", &Font8, COLORED);
			drawUpdate();
		}
		else
		{

			y_dis += 10;
			pt.DrawStringAt(10, y_dis, "Server not available. Restart..", &Font8, COLORED);
			drawUpdate();
			WiFi.disconnect();
			delay(6000);
			ESP.restart();
		}
	}
	y_dis += 10;
	String ip = "IP: " + String(WiFi.localIP());
	pt.DrawStringAt(10, y_dis, ip.c_str(), &Font8, COLORED);

	if (WiFi.status() == WL_CONNECTED)
	{
		updateInfo inf = OTADRIVE.updateFirmwareInfo();

		String ver_now = "Version now: " + String(FIRMVERS);
		y_dis += 10;
		pt.DrawStringAt(10, y_dis, ver_now.c_str(), &Font8, COLORED);

		ver_now = "New Version: " + inf.version;
		y_dis += 10;
		pt.DrawStringAt(10, y_dis, ver_now.c_str(), &Font8, COLORED);

		drawUpdate();

		if (inf.available)
		{
			String start_now = "Start update..." + inf.version;
			y_dis += 10;
			pt.DrawStringAt(10, y_dis, start_now.c_str(), &Font8, COLORED);
			y_dis += 10;
			OTADRIVE.updateFirmware();
		}
	}

	WiFi.disconnect();
	ESP.restart();
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

	menuCount = countTagsNow;
	String printtags;
	for (int idx = 0; idx <= 10; idx++)
	{
		printtags = AllTag[countTags - 1 - idx].tag;
		pt.DrawStringAt(25, 25 + (idx * 15), printtags.c_str(), &Font12, COLORED);
	}
	return 0;
}

int MenuHistory(int menuNow)
{
	for (int i = 0; i < 4; i++)
	{
		menuHis[i] = menuHis[i + 1];
	}
	menuHis[4] = menuNow;
	menuLast = menuNow;
	return 0;
}

int symHistory(char symNow)
{
	for (int i = 0; i < 4; i++)
	{
		symHis[i] = symHis[i + 1];
	}
	symHis[4] = symNow;
	symLast = symNow;
	return 0;
}

bool caseUpperFlag = false;
uint8_t keyboardColl = 0, keyboardRow = 0;
char checkButton()
{
	keyboardColl = 0;
	keyboardRow = 0;
	if (key_matrix.update())
	{
		for (int i = 0; i < key_matrix.pressed_key_count; i++)
		{
			keyboardRow = key_matrix.pressed_list[i].row;
			keyboardColl = key_matrix.pressed_list[i].col;
		}
	}
	else
	{
		return '~';
	}

	if (arrSymbol[keyboardRow][keyboardColl] != '0')
	{
		if (keyboardRow == 6 && keyboardColl == 2 && symLast == CTRL)
		{
			if (keyboardUpperFlag)
			{
				keyboardUpperFlag = false;
				LedUpperCase(false);
			}
			else
			{
				keyboardUpperFlag = true;
				LedUpperCase(true);
			}

			symHistory('~');
			return '~';
		}
		else if (keyboardRow == 6 && keyboardColl == 2)
		{
			symHistory(CTRL);
		}
		else if (symLast == CTRL)
		{
			symHistory(arrCtrlSymbol[keyboardRow][keyboardColl]);
			return arrCtrlSymbol[keyboardRow][keyboardColl];
		}
		else if (keyboardUpperFlag)
		{

			symHistory(arrUpperSymbol[keyboardRow][keyboardColl]);
			return arrUpperSymbol[keyboardRow][keyboardColl];
		}
		else
		{
			symHistory(arrSymbol[keyboardRow][keyboardColl]);
			return arrSymbol[keyboardRow][keyboardColl];
		}
	}
	else
	{
		return '~';
	}

	return '~';
}
int loraSendConfirm(unsigned long userMesh, String messKey)
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
			int MsLen = msgData.length() + 1;
			msgData.getBytes(outData.data, MsLen);
			int OutLen = sizeof(outData.data) / sizeof(outData.data[0]);
			int dataLen = MAP_HEADER_SIZE + OutLen;

			if (!addSendRequest(&outData, dataLen))
			{
#ifdef logc
				Serial.println("Send CONFIRM fail");
#endif
			}
			else
			{
#ifdef logc
				Serial.println("Send CONFIRM - Ok");
#endif
			}
		}
		else
		{
			xSemaphoreGive(accessNodeList);
#ifdef logc
			Serial.println("Not enough nodes in the list in CONFIRM");
#endif
		}
	}
	else
	{
#ifdef logc
		Serial.println("Could not access the nodes list in CONFIRM");
#endif
	}

	return 0;
}

int loraSendMessage(unsigned long userMesh, String msgData)
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
#ifdef logc
				Serial.println("routeToNode.firstHop!=0");
				Serial.print("outData.dest  ");
				Serial.println(outData.dest);
				Serial.print("outData.from  ");
				Serial.println(outData.from);
				Serial.print("outData.type  ");
				Serial.println(outData.type);

				Serial.printf("Queuing msg to hop to %08X over %08X\n", outData.from, outData.dest);
#endif
			}
			else
			{
				outData.dest = routeToNode.nodeId;
				outData.from = deviceID;
				outData.orig = deviceID;
				outData.type = LORA_DIRECT;
#ifdef logc
				Serial.println("routeToNode.firstHop>0");
				Serial.print("outData.dest  ");
				Serial.println(outData.dest);
				Serial.print("outData.from  ");
				Serial.println(outData.from);
				Serial.print("outData.type  ");
				Serial.println(outData.type);
				Serial.printf("Queuing msg direct to %08X\n", outData.dest);
#endif
			}

			String key = String(deviceID);
			char DeviceIDmac[9];
			itoa(deviceID, DeviceIDmac, 16);
			String output = String(DeviceIDmac);
			String messKey = output.substring(3, 7) + random(10000, 90000);

			msgData = "~" + messKey + "~" + msgData;
			int MsLen = msgData.length() + 1;
			msgData.getBytes(outData.data, MsLen);
			int OutLen = sizeof(outData.data) / sizeof(outData.data[0]);
			int dataLen = MAP_HEADER_SIZE + OutLen;

			if (!addSendRequest(&outData, dataLen))
			{
				LedLoraWarning(false);
#ifdef logc
				Serial.println("Send fail");
#endif
			}
			else
			{
#ifdef logc
				Serial.println("Send - Ok");
#endif
				db_addIndMessage(13, userMesh, msgData, 0, 0, 0);
				LedLoraWarning(true);
			}
		}
		else
		{
			xSemaphoreGive(accessNodeList);
#ifdef logc
			Serial.println("Not enough nodes in the list");
#endif
		}
	}
	else
	{
#ifdef logc
		Serial.println("Could not access the nodes list");
#endif
	}
	return 0;
}

int loraSendBroadcast(String BroadMessage)
{
	outData.mark1 = 'L';
	outData.mark2 = 'o';
	outData.mark3 = 'R';
	getNextBroadcastID();

	outData.dest = getNextBroadcastID();
	outData.from = deviceID;
	outData.type = LORA_BROADCAST;

	int MsLen = BroadMessage.length();
	BroadMessage.getBytes(outData.data, MsLen + 1);
	int OutLen = sizeof(outData.data) / sizeof(outData.data[0]);

	int dataLen = MAP_HEADER_SIZE + OutLen;
	if (!addSendRequest(&outData, dataLen))
	{
		LedLoraWarning(false);
#ifdef logc
		Serial.println("Sending BR fail");
#endif
		return 1;
	}
	else
	{
#ifdef logc
		Serial.println("Sending - OK");
#endif
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
	for (int g = countTags; g >= 150; g--)
	{
		Serial.print(g);
		Serial.print(": ");
		Serial.print(AllTag[g].active);
		Serial.print(" - ");
		Serial.print(AllTag[g].counter);
		Serial.print(" - ");
		Serial.print(AllTag[g].tag);
	}
	return 0;
}

int noUsersWindow()
{
	pt.DrawRectangle(25, 55, 175, 100, COLORED);
	pt.DrawFilledRectangle(20, 60, 180, 95, UNCOLORED);
	pt.DrawStringAt(27, 65, "No users. Press Menu", &Font12, COLORED);
	pt.DrawStringAt(27, 80, "Next update: 30 sec", &Font12, COLORED);

	return 0;
}

int db_printGenALL()
{
	for (int g = 1000; g >= 980; g--)
	{
		Serial.print(g);
		Serial.print(": ");
		Serial.print(GenChat[g].OnLora_fromID);
		Serial.print(" - ");
		Serial.print(GenChat[g].message);
		Serial.print(" - ");
		Serial.print(GenChat[g].OnLora_rxRssi);
		Serial.print(" - ");
		Serial.print(GenChat[g].OnLora_rxSize);
		Serial.print(" - ");
		Serial.println(GenChat[g].OnLora_rxSnr);
	}
	return 0;
}

int db_printIndALL()
{
	for (int g = 30; g >= 0; g--)
	{
		Serial.print(g);
		Serial.print(": ");
		Serial.print(IndChat[g].OnLora_fromID);
		Serial.print(" - ");
		Serial.print(IndChat[g].OnLora_toID);
		Serial.print(" - ");
		Serial.print(IndChat[g].message);
		Serial.print(" - ");
		Serial.print(IndChat[g].OnLora_rxRssi);
		Serial.print(" - ");
		Serial.print(IndChat[g].OnLora_rxSize);
		Serial.print(" - ");
		Serial.print(IndChat[g].OnLora_rxSnr);
		Serial.print(" - ");
		Serial.print((int)IndChat[g].confirm);
		Serial.print(" - ");
		Serial.println(IndChat[g].confirmCode);
	}
	return 0;
}

int db_addTag(String tagNow)
{
	for (int i = 0; i < countTags - 1; i++) // for(auto const i = &x) size_t i = 0;
	{
		if (tagNow == AllTag[i].tag)
		{
#ifdef logc
			Serial.println("Tag already exist");
#endif
			return 0;
		}
	}
	for (int i = 0; i < countTags - 1; i++)
	{
		AllTag[i].active = AllTag[i + 1].active;
		AllTag[i].counter = AllTag[i + 1].counter;
		AllTag[i].tag = AllTag[i + 1].tag;
	}
	AllTag[countTags - 1].tag = tagNow;
#ifdef logc
	Serial.println("Tag added");
#endif
	return 0;
}

int db_addGenMessage(uint32_t dbfromID, String dbrxPayload, uint16_t dbrxSize, int16_t dbxRssi, int8_t dbrxSnr)
{
	for (int i = 0; i < countMessage - 1; i++)
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
#ifdef logc
	Serial.println("Message added");
#endif

	uint8_t spacepos = dbrxPayload.indexOf(" ");
	if (spacepos != -1 && dbrxPayload.charAt(0) == '#')
	{
		db_addTag(dbrxPayload.substring(0, spacepos));
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

#ifdef logc
	Serial.println("Message ind added");
	db_printIndALL();
#endif

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
	menuCount = 6;
	for (int i = 0; i <= menuCount; i++)
	{
		pt.DrawStringAt(20, 25 + (i * 15), menuAll[i].itemName, &Font12, COLORED);
	}

	char DeviceIDmac[9];
	itoa(deviceID, DeviceIDmac, 16);
	String MyNode = "My m.id:" + String(DeviceIDmac) + " / Ver:" + String(FIRMVERS);
	pt.DrawStringAt(20, 165, MyNode.c_str(), &Font8, COLORED);

	return 0;
}

int MenuDrawArow(int selected)
{
	pt.DrawFilledRectangle(0, 24, 19, 200, UNCOLORED);
	pt.DrawStringAt(3, 25 + (selected * 15), "->", &Font12, COLORED);

	return 0;
}

int MenuDrawDistance()
{
	pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);
	pt.DrawVerticalLine(10, 25, 160, COLORED);
	pt.DrawHorizontalLine(10, 185, 170, COLORED);

	pt.DrawFilledCircle(10, 185, 7, COLORED);
	pt.DrawCircle(10, 185, 9, COLORED);
	pt.DrawCircle(10, 185, 12, COLORED);

	pt.DrawStringAt(3, 60, "S", &Font8, COLORED);
	pt.DrawStringAt(3, 69, "N", &Font8, COLORED);
	pt.DrawStringAt(3, 78, "R", &Font8, COLORED);

	pt.DrawStringAt(100, 190, "RSSI", &Font8, COLORED);

	for (int g = 0; g < numElements; g++)
	{
		if (abs(dnode[g].rssi) < 115 && abs(dnode[g].snr) < 7) // good
		{
			pt.DrawCircle(random(10, 100), random(100, 170), 6, COLORED);
		}
		else if ((abs(dnode[g].rssi) >= 115 && abs(dnode[g].rssi) <= 126 && abs(dnode[g].snr) <= 7) || (abs(dnode[g].rssi) <= 115 && abs(dnode[g].snr) >= 7 && abs(dnode[g].snr) <= 15)) // fair
		{
			pt.DrawCircle(random(10, 140), random(60, 100), 4, COLORED);
		}
		else if ((abs(dnode[g].rssi) <= 126 && abs(dnode[g].snr) > 15) || (abs(dnode[g].rssi) > 126 && abs(dnode[g].snr) < 15)) // bad
		{
			pt.DrawCircle(random(150, 170), random(30, 150), 2, COLORED);
		}
		else // error
		{
			pt.DrawCircle(random(15, 170), random(30, 185), 1, COLORED);
		}
	}
	return 0;
}

int MenuDrawLoraSet()
{
	pt.DrawFilledRectangle(0, 22, 200, 200, UNCOLORED);
	String RF_F = "FREQUENCY: " + String(RF_FREQUENCY);
	String TX_O = "OUTPUT_POWER: " + String(TX_OUTPUT_POWER);
	String LORA_B = "BANDWIDTH: " + String(LORA_BANDWIDTH);
	String LORA_S = "SPREADING_FACTOR: " + String(LORA_SPREADING_FACTOR);
	String LORA_C = "CODINGRATE: " + String(LORA_CODINGRATE);
	String LORA_P = "PREAMBLE_LENGTH: " + String(LORA_PREAMBLE_LENGTH);
	String LORA_Sym = "SYMBOL_TIMEOUT: " + String(LORA_SYMBOL_TIMEOUT);
	// bool LORA_F = bool(LORA_FIX_LENGTH_PAYLOAD_ON);
	// bool LORA_I = bool(LORA_IQ_INVERSION_ON);

	pt.DrawStringAt(10, 30, RF_F.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 45, TX_O.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 60, LORA_B.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 75, LORA_S.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 90, LORA_C.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 105, LORA_P.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 120, LORA_Sym.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 120, LORA_Sym.c_str(), &Font12, COLORED);

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

	String batterystate = String(map(adcValue, 2827, 4095, 0, 100)) + "%";
#ifdef logc
	Serial.print("Battery: ");
	Serial.println(batterystate);
#endif
	return batterystate;
}

String getVoltage()
{
	int adcValue = analogRead(PinBattery);
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
	return 0;
}

int MenuDrawStatDb()
{
	pt.DrawFilledRectangle(0, 0, 200, 18, UNCOLORED);

	pt.DrawHorizontalLine(0, 20, 200, COLORED);

	return 0;
}

int MenuAllUserDraw()
{
	uint8_t xcoor = 20;
	pt.DrawFilledRectangle(0, 23, 200, 200, UNCOLORED);
	menuCount = numElements;

	if (numElements == 0)
	{
		noUsersWindow();
	}
	else
	{
		char line[128];
		for (int idx = 0; idx < numElements; idx++)
		{
			if (firstHop[idx] == 0)
			{
				sprintf(line, "%02X%02X%02X", (uint8_t)(nodeId[idx] >> 24), (uint8_t)(nodeId[idx] >> 16), (uint8_t)(nodeId[idx] >> 8));
			}
			else
			{
				sprintf(line, "%02X%02X%02X*", (uint8_t)(nodeId[idx] >> 24), (uint8_t)(nodeId[idx] >> 16), (uint8_t)(nodeId[idx] >> 8));
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

int chatSingleDraw(unsigned int activeUserId, uint8_t Upd = 0)
{

	pt.DrawFilledRectangle(0, 11, 200, 200, UNCOLORED);
	uint8_t ycoordinates = 165;

	String msgPrint, who;

	if (numElements == 0)
	{
		noUsersWindow();
	}
	else
	{
		String inkStr[countinskStr];
		for (int g = 0; g < countinskStr; g++)
		{
			inkStr[g] = "";
		}
		// Берем каждое сообщение
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
					who = "bff:";
				}

				if (IndChat[t].confirm)
				{

					who += "> ";
				}
				else
				{

					who += "- ";
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
		noUsersWindow();
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
	epd154bw.SetFrameMemory(pt.GetImage(), 0, 0, pt.GetWidth(), pt.GetHeight());
	epd154bw.HalLcd_Partial_Update();
	return 0;
}

int DrawHistory()
{
	for (int i = 0; i <= 4; i++)
	{
		Serial.print(i);
		Serial.print(" - ");
		Serial.print(symHis[i]);
	}

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

	// epd154bw.SetFrameMemory(ptChat.GetImage(), 3, 181, 190, 16);
	// epd154bw.HalLcd_Partial_Update();
	// drawUpdate();

	return 0;
}

int PrintFreeHeap()
{
	Serial.printf("\nHeap size: %d\n", ESP.getHeapSize());
	Serial.printf("Free Heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	Serial.printf("Min Free Heap: %d\n", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
	Serial.printf("Max Alloc Heap: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
	return 0;
}

void setup()
{

	pref.begin("Update", false);
	stateUpdate = pref.getBool("state", false);
	pref.end();

	if (stateUpdate == false)
	{
		setCpuFrequencyMhz(80);
	}
	else
	{
		setCpuFrequencyMhz(240);
	}

#ifdef logc
	Serial.begin(115200);
	Serial.print("Firmware version: ");
	Serial.println(FIRMVERS);
#endif

	OTADRIVE.setInfo(APIKEY, FIRMVERS);
	OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);
	LedSystemStart();

#ifdef logc

	int freq = millis();
	Serial.println(millis() - freq);
	uint32_t Freq = getCpuFrequencyMhz();
	Serial.print("CPU Freq = ");
	Serial.print(Freq);
	Serial.println(" MHz");
	Freq = getXtalFrequencyMhz();
	Serial.print("XTAL Freq = ");
	Serial.print(Freq);
	Serial.println(" MHz");
	Freq = getApbFrequency();
	Serial.print("APB Freq = ");
	Serial.print(Freq);
	Serial.println(" Hz");

#endif

	epd154bw.EPD_Init();
	delay(200);
	epd154bw.DisplayPartBaseImage(gImage_backg);
	delay(200);
	epd154bw.HalLcd_Partial();
	delay(200);

	pt.SetWidth(200);
	pt.SetHeight(200);
	pt.Clear(UNCOLORED);
	pt.SetRotate(ROTATE_270);

	ptChat.SetWidth(200);
	ptChat.SetHeight(14);
	ptChat.Clear(UNCOLORED);
	ptChat.SetRotate(ROTATE_270);

	MenuHeader("LoraType");

	Wire.begin(PinSDA, PinSCL, 100000);
	delay(200);

	key_matrix.begin();

	if (stateUpdate)
	{
		pref.begin("Update");
		pref.putBool("state", false);
		pref.end();
		OtaUpdate();
	}

	pt.DrawStringAt(30, 45, "LoraType..|", &Font20, COLORED);

	/*
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
		pt.DrawStringAt(10, 80, "Welcome!", &Font12, COLORED);
		*/

	/*==============================Lora=======================*/
	uint8_t deviceMac[8];
	BoardGetUniqueId(deviceMac);
	deviceID += (uint32_t)deviceMac[2] << 24;
	deviceID += (uint32_t)deviceMac[3] << 16;
	deviceID += (uint32_t)deviceMac[4] << 8;
	deviceID += (uint32_t)deviceMac[5];
	myLog_n("Mesh NodeId = %08lX", deviceID);
	char DeviceIDmac[9];
	itoa(deviceID, DeviceIDmac, 16);
	String output = "Mesh NodeId: " + String(DeviceIDmac);

	pt.DrawStringAt(10, 125, output.c_str(), &Font12, COLORED);

	if (!initLoRa())
	{
		pt.DrawStringAt(10, 140, "LoRa init - failed", &Font12, COLORED);
#ifdef logc
		Serial.println("LoRa init - failed");
#endif
	}
	else
	{
		pt.DrawStringAt(10, 140, "LoRa initialization - Ok", &Font12, COLORED);

#ifdef logc
		Serial.println("LoRa initialization - Ok");
#endif
	}

	output = "CpuFrequency: " + String(getCpuFrequencyMhz());
	pt.DrawStringAt(10, 95, output.c_str(), &Font12, COLORED);
	output = "Firmware Ver.:" + String(FIRMVERS);
	pt.DrawStringAt(10, 110, output.c_str(), &Font12, COLORED);
	output = "Battery charge: " + getBattery();
	pt.DrawStringAt(10, 155, output.c_str(), &Font12, COLORED);
	output = "Voltage: " + getVoltage() + " v";
	pt.DrawStringAt(10, 170, output.c_str(), &Font12, COLORED);
	pt.DrawStringAt(10, 185, "Let's start.. Press MENU", &Font12, COLORED);

	drawUpdate();
	delay(100);
	sendRandom = millis();
	itwork = sendRandom;
}

void loop()
{
	if (OnLoraFlag)
	{
		OnLoraFlag = false;

		if (OnLora_rxPayload[0] == '@')
		{
			OnLora_rxPayload.remove(0, 1);

			for (int i = 0; i < countMessage; i++)
			{
				if (OnLora_rxPayload == IndChat[i].confirmCode)
				{

					IndChat[i].confirm = true;
					break;
				}
			}
		}
		else if (OnLora_rxPayload[0] == '~')
		{

			db_addIndMessage(OnLora_rxfromID, 13, OnLora_rxPayload, OnLora_rxSize, OnLora_rxRssi, OnLora_rxSnr);
			delay(500);
			loraSendConfirm(OnLora_rxfromID, getConfirmCode(OnLora_rxPayload));
			LedLoraInputMess(true);
		}
		else
		{
			db_addGenMessage(OnLora_rxfromID, OnLora_rxPayload, OnLora_rxSize, OnLora_rxRssi, OnLora_rxSnr);
			LedLoraInputMess(false);
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

	sym = checkButton();

	if (sym != '~')
	{
		symHistory(sym);
		if ((isIndChat && ((int)sym >= 32)) || (isGenChat && ((int)sym >= 32)))
		{
			outMessage += sym;
			chatDrawOutmess();
			drawUpdate();
		}

		if (sym == SPACE)
		{
			outMessage += " ";
			chatDrawOutmess();
			drawUpdate();
		}

		if (isGenChat && sym == ENTER)
		{

			if (outMessage.length() >= 3)
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

				MenuHistory(8);
				counterResMess = 0;
				drawUpdate();
			}
			else
			{

				LedSystemWarning();
			}
		}
		if (sym == LEFT)
		{
			menuCount = 6;
			menuNowSelect = menuHis[4];
			sym = symHis[4];
		}

		if (sym == MENU)
		{
			menuCount = sizeof(menuAll) / sizeof(menuAll[0]);

			MenuHistory(0);
			symHistory(sym);
			menuNowSelect = 0;
			isIndChat = false;
			isGenChat = false;
			outMessage = "";
			MenuHeader("Menu");
			MenuDraw();
			MenuDrawArow(0);
			drawUpdate();
		}

		if (sym == UP && menuLast == 0 && menuNowSelect > 0)
		{
			menuNowSelect -= 1;

			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}

		if (sym == DOWN && menuLast == 0 && menuNowSelect <= menuCount - 1)
		{
			menuNowSelect += 1;
			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 0)
		{
			MenuHistory(0);
			symHistory(sym);
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			isIndChat = false;
			isGenChat = true;
			chatGenDraw();
			drawUpdate();
			sym = '~';
		}

		if (sym == UP && isGenChat)
		{
			if (messFrame < countinskStr && (allstrCounter - 10) > 1)
			{
				messFrame++;

				chatGenDraw(messFrame);
				drawUpdate();
			}
			sym = '~';
		}

		if (sym == DOWN && isGenChat)
		{
			if (messFrame > 0)
			{
				messFrame--;
				chatGenDraw(messFrame);
				drawUpdate();
			}
			sym = '~';
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 1)
		{
			MenuHistory(1);
			symHistory(sym);
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			MenuAllUserDraw();
			if (numElements > 0)
			{
				MenuDrawArow(0);
			}
			drawUpdate();
			menuNowSelect = 0;
			sym = '~';
		}

		if (sym == UP && menuLast == 1 && menuNowSelect >= 0)
		{
			menuNowSelect -= 1;
			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}
		if (sym == DOWN && menuLast == 1 && menuNowSelect <= menuCount - 1)
		{
			menuNowSelect += 1;

			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}

		if (sym == OK && menuLast == 1)
		{
			isGenChat = false;
			isIndChat = true;
			MenuHistory(7);
			symHistory(sym);
			getActiveUser = nodeId[menuNowSelect];
			MenuHeader(String(getActiveUser, HEX));
			chatSingleDraw(getActiveUser);
			drawUpdate();
		}

		if (isIndChat && sym == ENTER)
		{
			if (outMessage.length() >= 3)
			{
				loraSendMessage(getActiveUser, outMessage);
				outMessage = "";
				counterResMess = 0;
				pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
				chatSingleDraw(getActiveUser);
				MenuHistory(7);
				drawUpdate();
			}
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 2)
		{
			MenuHistory(2);
			symHistory(sym);
			isGenChat = false;
			isIndChat = false;

			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			MenuAllTagsDraw();
			drawUpdate();
			menuNowSelect = 0;
			sym = '~';
		}

		if (sym == UP && menuLast == 2 && menuNowSelect >= 0)
		{
			menuNowSelect -= 1;
			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}

		if (sym == DOWN && menuLast == 2 && menuNowSelect <= menuCount - 1)
		{
			menuNowSelect += 1;
			MenuDrawArow(menuNowSelect);
			drawUpdate();
		}

		if (sym == UP && isTagsView)
		{
			if (messFrame < countinskStr && (allstrCounter - 10) > 1)
			{
				messFrame++;
				tagSingleDraw(getActiveTag, messFrame);
				drawUpdate();
			}
			sym = '~';
		}

		if (sym == DOWN && isTagsView)
		{
			if (messFrame > 0)
			{
				messFrame--;

				tagSingleDraw(getActiveTag, messFrame);
				drawUpdate();
			}
			sym = '~';
		}

		if (sym == OK && menuLast == 2)
		{
			MenuHistory(9);
			symHistory(sym);
			isTagsView = true;
			getActiveTag = AllTag[menuNowSelect].tag;

			counterResMess = 0;
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			tagSingleDraw(getActiveTag);
			drawUpdate();
			menuNowSelect = 0;

			drawUpdate();
			sym = '~';
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 6)
		{
			MenuHistory(3);
			symHistory(sym);
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			drawAbout();
			drawUpdate();
			sym = '~';
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 3)
		{
			MenuHistory(4);
			symHistory(sym);
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			MenuDrawLoraSet();
			drawUpdate();
			sym = '~';
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 4)
		{
			MenuHistory(5);
			symHistory(sym);
			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			MenuDrawDistance();
			drawUpdate();
			sym = '~';
		}

		if (sym == OK && menuLast == 0 && menuNowSelect == 5)
		{

			MenuHistory(6);
			symHistory(sym);

			String headerName = String(menuAll[menuNowSelect].itemName);
			MenuHeader(headerName);
			menuNowSelect = 0;
			pref.begin("Update", false);
			pref.putBool("state", true);
			pref.end();
#ifdef logc
			Serial.println("State Update = true");
#endif

			drawUpdate();
			delay(1000);
			ESP.restart();
			sym = '~';
		}

		if (sym == BACKSPACE)
		{
			outMessage.remove(outMessage.length() - 1, 1);
			chatDrawOutmess();
		}

		sym = '~';
	}

	/*===============================Self-messaging===========================*/
	/*
		if (numElements > 0 && isGenChat && (millis() - sendBrodcast) >= (30000 + random(100, 5000)))
		{
			loraSendBroadcast(selfText[random(0, 19)]);
			chatGenDraw();
			drawUpdate();
			sendBrodcast = millis();
		}
	*/
	if ((millis() - sendRandom) >= 15000)
	{
		if (CounterUsersUpdater())
		{
			drawUpdate();
		}
		sendRandom = millis();
	}

	if ((millis() - itwork) >= 10000)
	{
		heartLed();
		// Bad idea
		//  pt.DrawCircle(194, 17, 3, COLORED);
		//  pt.DrawFilledCircle(194, 17, 2, (int)workFlag);
		//  workFlag = !workFlag;
		//  drawUpdate();
		itwork = millis();
	}
}
