#include "main.h"

/** Task to handle mesh */
TaskHandle_t meshTaskHandle = NULL;

/** Queue to handle cloud send requests */
volatile xQueueHandle meshMsgQueue;

/** Counter for CAD retry */
uint8_t channelFreeRetryNum = 0;

/** The Mesh node ID, created from ID of the nRF52 */
uint32_t deviceID;
/** The Mesh broadcast ID, created from node ID */
uint32_t broadcastID;

/** Map message buffer */
mapMsg syncMsg;

/** Max number of messages in the queue */
#define SEND_QUEUE_SIZE 2 // 2
/** Send buffer for SEND_QUEUE_SIZE messages */
dataMsg sendMsg[SEND_QUEUE_SIZE];
/** Message size buffer for SEND_QUEUE_SIZE messages */
uint8_t sendMsgSize[SEND_QUEUE_SIZE];
/** Queue to handle send requests */
volatile xQueueHandle sendQueue;
#ifdef ESP32
/** Mux used to enter critical code part (clear up queue content) */
portMUX_TYPE accessMsgQueue = portMUX_INITIALIZER_UNLOCKED;
#endif
/** Mux used to enter critical code part (access to node list) */
SemaphoreHandle_t accessNodeList;

/** LoRa TX package */
uint8_t txPckg[256]; // 256
/** Size of data package */
uint16_t txLen = 0;
/** LoRa RX buffer */
uint8_t rxBuffer[256]; // 256

/** Sync time for routing at start */
#define INIT_SYNCTIME 30000
/** Sync time for routing after mesh has settled */
#define DEFAULT_SYNCTIME 60000 // 60000
/** Time to switch from INIT_SYNCTIME to DEFAULT_SYNCTIME */
#define SWITCH_SYNCTIME 300000
/** Sync time */
time_t syncTime = INIT_SYNCTIME;

/**
 * Sleep and Listen time definitions
 * Calculated with Semtech SX1261 Calculater
 * SF 7, BW 250, CR 4/5 PreambleLen 8 PayloadLen 253 Hdr enabled CRC enabled
 * 2 -> number of preambles required to detect package
 * 512 -> length of a symbol im ms
 * 1000 -> wake time is in us
 * 15.625 -> SX126x counts in increments of 15.625 us
 *
 * 10 -> max length we can sleep in symbols
 * 512 -> length of a symbol im ms
 * 1000 -> sleep time is in us
 * 15.625 -> SX126x counts in increments of 15.625 us
 */
// #define RX_SLEEP_TIMES 2 * 512 * 1000 * 15.625, 10 * 512 * 1000 * 15.625

#define RX_SLEEP_TIMES 2 * 4096 * 1000 * 15.625, 10 * 4096 * 1000 * 15.625

typedef enum
{
	MESH_IDLE = 0, //!< The radio is idle
	MESH_RX,	   //!< The radio is in reception state
	MESH_TX,	   //!< The radio is in transmission state
	MESH_NOTIF	   //!< The radio is doing mesh notification
} meshRadioState_t;

/** Lora statemachine status */
meshRadioState_t loraState = MESH_IDLE;

/** LoRa callback events */
static RadioEvents_t RadioEvents;

/** Mesh callback variable */
static MeshEvents_t *_MeshEvents;

/** Number of nodes in the map */
int _numOfNodes = 0;

/** Timeout for RX after Preamble detection */
time_t preambTimeout;

/** Flag if the nodes map has changed */
boolean nodesChanged = false;

/**
 * Initialize the Mesh network
 * @param events
 * 		Structure of event callbacks
 * @param numOfNodes
 * 		Number of nodes that the Mesh network can accept.
 */
void initMesh(MeshEvents_t *events, int numOfNodes)
{


	_MeshEvents = events;
	// Initialize the callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;
	// RadioEvents.PreAmpDetect = OnPreAmbDetect;
	Radio.Init(&RadioEvents);

	_numOfNodes = numOfNodes;

	// Prepare empty nodes map
	nodesMap = (nodesList *)malloc(_numOfNodes * sizeof(nodesList));

	if (nodesMap == NULL)
	{
		myLog_e("Could not allocate memory for nodes map");
	}
	else
	{
		myLog_d("Memory for nodes map is allocated");
	}
	memset(nodesMap, 0, _numOfNodes * sizeof(nodesList));

	// // Prepare empty names map
	// namesMap = (namesList *)malloc(_numOfNodes * sizeof(namesList));

	// if (namesMap == NULL)
	// {
	// 	myLog_e("Could not allocate memory for names map");
	// }
	// else
	// {
	// 	myLog_d("Memory for names map is allocated");
	// }
	// memset(namesMap, 0, _numOfNodes * sizeof(namesList));

	// Create queue
	sendQueue = xQueueCreate(SEND_QUEUE_SIZE, sizeof(uint8_t));
	if (sendQueue == NULL)
	{
		myLog_e("Could not create send queue!");
	}
	else
	{
		myLog_d("Send queue created!");
	}
	// Create blocking semaphore for nodes list access
	accessNodeList = xSemaphoreCreateBinary();
	xSemaphoreGive(accessNodeList);

	// Create broadcast ID
	broadcastID = deviceID & 0xFFFFFF00;
	myLog_d("Broadcast ID is %08X", broadcastID);

	int32_t set_rf_frequency=0;
	int32_t set_tx_output_power = 0;			
	int32_t set_lora_spreading_factor = 0;			
	int32_t set_lora_codingrate = 0;			
	int32_t set_lora_bandwidth = 0;		

	Preferences prefLora;
	prefLora.begin("LoraSettings", false);

	set_rf_frequency = prefLora.getInt("rf_frequency", 868200000);		  // 
	set_tx_output_power = prefLora.getInt("tx_output_power", 14);				  // tx_output_power=14;
	set_lora_spreading_factor = prefLora.getInt("lora_spreading", 12); // lora_spreading_factor=12
	set_lora_codingrate = prefLora.getInt("lora_codingrate", 1);			  // lora_codingrate=12
	set_lora_bandwidth = prefLora.getInt("lora_bandwidth", 1);				  // lora_bandwidth=1

	prefLora.end();



	// Put LoRa into standby
	Radio.Standby();

	// Set Frequency
	Radio.SetChannel(set_rf_frequency);

	// Set transmit configuration
	Radio.SetTxConfig(MODEM_LORA, set_tx_output_power, 0, set_lora_bandwidth,
					  set_lora_spreading_factor, set_lora_codingrate,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
	// Set receive configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	// Create message queue for LoRa
	meshMsgQueue = xQueueCreate(10, sizeof(uint8_t));
	if (meshMsgQueue == NULL)
	{
		myLog_e("Could not create LoRa message queue!");
	}
	else
	{
		myLog_d("LoRa message queue created!");
	}

	if (!xTaskCreate(meshTask, "MeshSync", 3096, NULL, 1, &meshTaskHandle))
	{
		myLog_e("Starting Mesh Sync Task failed");
	}
	else
	{
		myLog_d("Starting Mesh Sync Task success");
	}
}

/**
 * Task to handle the mesh
 * @param pvParameters
 * 		Unused task parameters
 */
void meshTask(void *pvParameters)
{
	// Queue variable to be sent to the task
	uint8_t queueIndex;

	time_t notifyTimer = millis() + syncTime;
	// time_t cleanTimer = millis();
	time_t checkSwitchSyncTime = millis();

	loraState = MESH_IDLE;
	// Start waiting for data package
	Radio.Standby();
	// Radio.Rx(0);
	Radio.SetRxDutyCycle(RX_SLEEP_TIMES);

	time_t txTimeout = millis();

	while (1)
	{
		Radio.IrqProcess();

		if (nodesChanged)
		{
			nodesChanged = false;
			if ((_MeshEvents != NULL) && (_MeshEvents->NodesListChanged != NULL))
			{
				_MeshEvents->NodesListChanged();
			}
		}
		// Time to sync the Mesh ???
		if ((millis() - notifyTimer) >= syncTime)
		{
			if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
			{
				myLog_v("Checking mesh map");
				if (!cleanMap())
				{
					syncTime = INIT_SYNCTIME;
					checkSwitchSyncTime = millis();
					if ((_MeshEvents != NULL) && (_MeshEvents->NodesListChanged != NULL))
					{
						_MeshEvents->NodesListChanged();
					}
				}
				myLog_d("Sending mesh map");
				syncMsg.from = deviceID;
				syncMsg.type = LORA_NODEMAP;
				memset(syncMsg.nodes, 0, 48 * 5);

				// Get sub nodes
				uint8_t subsLen = nodeMap(syncMsg.nodes);

				xSemaphoreGive(accessNodeList);

				if (subsLen != 0)
				{
					for (int idx = 0; idx < subsLen; idx++)
					{
						uint32_t checkNode = syncMsg.nodes[idx][0];
						checkNode |= syncMsg.nodes[idx][1] << 8;
						checkNode |= syncMsg.nodes[idx][2] << 16;
						checkNode |= syncMsg.nodes[idx][3] << 24;
					}
				}
				syncMsg.nodes[subsLen][0] = 0xAA;
				syncMsg.nodes[subsLen][1] = 0x55;
				syncMsg.nodes[subsLen][2] = 0x00;
				syncMsg.nodes[subsLen][3] = 0xFF;
				syncMsg.nodes[subsLen][4] = 0xAA;
				subsLen++;

				subsLen = MAP_HEADER_SIZE + (subsLen * 5);

				if (!addSendRequest((dataMsg *)&syncMsg, subsLen))
				{
					myLog_e("Cannot send map because send queue is full");
				}
				notifyTimer = millis();
			}
			else
			{
				myLog_e("Cannot access map for clean up and syncing");
			}
		}

		// Time to relax the syncing ???
		if (((millis() - checkSwitchSyncTime) >= SWITCH_SYNCTIME) && (syncTime != DEFAULT_SYNCTIME))
		{
			myLog_v("Switching sync time to DEFAULT_SYNCTIME");
			syncTime = DEFAULT_SYNCTIME;
			checkSwitchSyncTime = millis();
		}

		// Check if loraState is stuck in MESH_TX
		if ((loraState == MESH_TX) && ((millis() - txTimeout) > 7500))
		{
			Radio.Standby();
			// Radio.Rx(0);
			Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
			loraState = MESH_IDLE;
			myLog_e("loraState stuck in TX for 2 seconds");
		}

		// Check if we have something in the queue
		if (xQueuePeek(sendQueue, &queueIndex, (TickType_t)10) == pdTRUE)
		{
			if (loraState != MESH_TX)
			{
#ifdef ESP32
				portENTER_CRITICAL(&accessMsgQueue);
#else
				taskENTER_CRITICAL();
#endif
				txLen = sendMsgSize[queueIndex];
				memset(txPckg, 0, 256);
				memcpy(txPckg, &sendMsg[queueIndex].mark1, txLen);
				if (xQueueReceive(sendQueue, &queueIndex, portMAX_DELAY) == pdTRUE)
				{
					sendMsg[queueIndex].type = 0;
#ifdef ESP32
					portEXIT_CRITICAL(&accessMsgQueue);
#else
					taskEXIT_CRITICAL();
#endif

					myLog_d("Sending msg #%d with len %d", queueIndex, txLen);
					loraState = MESH_TX;
					Radio.Standby();
					Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
					// SX126xSetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
					// SX126xSetDioIrqParams(IRQ_RADIO_ALL,
					// 					  IRQ_RADIO_ALL,
					// 					  IRQ_RADIO_NONE, IRQ_RADIO_NONE);
					Radio.StartCad();
					txTimeout = millis();
				}
				else
				{
#ifdef ESP32
					portEXIT_CRITICAL(&accessMsgQueue);
#else
					taskEXIT_CRITICAL();
#endif
				}
			}
		}

		// Enable a task switch
		delay(100);
	}
}

/**
 * Callback after a LoRa package was received
 * @param rxPayload
 * 			Pointer to the received data
 * @param rxSize
 * 			Length of the received package
 * @param rxRssi
 * 			Signal strength while the package was received
 * @param rxSnr
 * 			Signal to noise ratio while the package was received
 */
void OnRxDone(uint8_t *rxPayload, uint16_t rxSize, int16_t rxRssi, int8_t rxSnr)
{
	// Secure buffer before restart listening
	if (rxSize < 256)
	{
		memcpy(rxBuffer, rxPayload, rxSize + 1);
		// Make sure the data is null terminated
		rxBuffer[rxSize] = 0;
	}
	else
	{
		memcpy(rxBuffer, rxPayload, rxSize);
	}

	uint16_t tempSize = rxSize;

	delay(1);

	loraState = MESH_IDLE;

	myLog_v("OnRxDone");
	myLog_d("LoRa Packet received size:%d, rssi:%d, snr:%d", rxSize, rxRssi, rxSnr);
#if MYLOG_LOG_LEVEL == MYLOG_LOG_LEVEL_VERBOSE
	for (int idx = 0; idx < rxSize; idx++)
	{
		Serial.printf(" %02X", rxBuffer[idx]);
	}
	Serial.println("");
#endif

	// Restart listening
	Radio.Standby();
	// Radio.Rx(0);
	Radio.SetRxDutyCycle(RX_SLEEP_TIMES);

	// Check the received data
	if ((rxBuffer[0] == 'L') && (rxBuffer[1] == 'o') && (rxBuffer[2] == 'R'))
	{
		// Valid Mesh data received
		mapMsg *thisMsg = (mapMsg *)rxBuffer;
		dataMsg *thisDataMsg = (dataMsg *)rxBuffer;

		if (thisMsg->type == LORA_NODEMAP)
		{
			/// \todo for debug make some nodes unreachable

			myLog_d("Got map message");
			// Mapping received
			uint8_t subsSize = tempSize - MAP_HEADER_SIZE;
			uint8_t numSubs = subsSize / 5;

			// Serial.println("********************************");
			// for (int idx = 0; idx < tempSize; idx++)
			// {
			// 	Serial.printf("%02X ", rxBuffer[idx]);
			// }
			// Serial.println("");
			// Serial.printf("subsSize %d -> # subs %d\n", subsSize, subsSize / 5);
			// Serial.println("********************************");

			// Check if end marker is in the message
			if ((thisMsg->nodes[numSubs - 1][0] != 0xAA) ||
				(thisMsg->nodes[numSubs - 1][1] != 0x55) ||
				(thisMsg->nodes[numSubs - 1][2] != 0x00) ||
				(thisMsg->nodes[numSubs - 1][3] != 0xFF) ||
				(thisMsg->nodes[numSubs - 1][4] != 0xAA))
			{
				myLog_e("Invalid map, end marker is missing from %08X", thisMsg->from);
				xSemaphoreGive(accessNodeList);
				return;
			}
			if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
			{
				nodesChanged = addNode(thisMsg->from, 0, 0);

				// Remove nodes that use sending node as hop
				clearSubs(thisMsg->from);

				myLog_v("From %08X", thisMsg->from);
				myLog_v("Dest %08X", thisMsg->dest);

				if (subsSize != 0)
				{
					// Mapping contains subs

					myLog_v("Msg size %d", tempSize);
					myLog_v("#subs %d", numSubs);

					// Serial.println("++++++++++++++++++++++++++++");
					// Serial.printf("From %08X Dest %08X #Subs %d\n", thisMsg->from, thisMsg->dest, numSubs);
					// for (int idx = 0; idx < numSubs; idx++)
					// {
					// 	uint32_t subId = (uint32_t)thisMsg->nodes[idx][0];
					// 	subId += (uint32_t)thisMsg->nodes[idx][1] << 8;
					// 	subId += (uint32_t)thisMsg->nodes[idx][2] << 16;
					// 	subId += (uint32_t)thisMsg->nodes[idx][3] << 24;
					// 	uint8_t hops = thisMsg->nodes[idx][4];
					// 	Serial.printf("ID: %08X numHops: %d\n", subId, hops);
					// }
					// Serial.println("++++++++++++++++++++++++++++");

					for (int idx = 0; idx < numSubs - 1; idx++)
					{
						uint32_t subId = (uint32_t)thisMsg->nodes[idx][0];
						subId += (uint32_t)thisMsg->nodes[idx][1] << 8;
						subId += (uint32_t)thisMsg->nodes[idx][2] << 16;
						subId += (uint32_t)thisMsg->nodes[idx][3] << 24;
						uint8_t hops = thisMsg->nodes[idx][4];
						if (subId != deviceID)
						{
							nodesChanged |= addNode(subId, thisMsg->from, hops + 1);
							myLog_v("Subs %08X", subId);
						}
					}
				}
				xSemaphoreGive(accessNodeList);
			}
			else
			{
				myLog_e("Could not access map to add node");
			}
		}
		else if (thisDataMsg->type == LORA_DIRECT)
		{
			if (thisDataMsg->dest == deviceID)
			{
				// Message is for us, call user callback to handle the data
				myLog_d("Got data message type %c >%s<", thisDataMsg->data[0], (char *)&thisDataMsg->data[1]);
				if ((_MeshEvents != NULL) && (_MeshEvents->DataAvailable != NULL))
				{
					_MeshEvents->DataAvailable(thisDataMsg->orig, thisDataMsg->data, tempSize - 12, rxRssi, rxSnr);
				}
			}
			else
			{
				// Message is not for us
			}
		}
		else if (thisDataMsg->type == LORA_FORWARD)
		{
			if (thisDataMsg->dest == deviceID)
			{
				// Message is for sub node, forward the message
				nodesList route;
				if (xSemaphoreTake(accessNodeList, (TickType_t)1000) == pdTRUE)
				{
					if (getRoute(thisDataMsg->from, &route))
					{
						// We found a route, send package to next hop
						if (route.firstHop == 0)
						{
							myLog_i("Route for %lX is direct", route.nodeId);
							// Destination is a direct
							thisDataMsg->dest = thisDataMsg->from;
							thisDataMsg->from = thisDataMsg->orig;
							thisDataMsg->type = LORA_DIRECT;
						}
						else
						{
							myLog_i("Route for %lX is to %lX", route.nodeId, route.firstHop);
							// Destination is a sub
							thisDataMsg->dest = route.firstHop;
							thisDataMsg->type = LORA_FORWARD;
						}

						// Put message into send queue
						if (!addSendRequest(thisDataMsg, tempSize))
						{
							myLog_e("Cannot forward message because send queue is full");
						}
					}
					else
					{
						myLog_e("No route found for %lX", thisMsg->from);
					}
					xSemaphoreGive(accessNodeList);
				}
				else
				{
					myLog_e("Could not access map to forward package");
				}
			}
			else
			{
				// Message is not for us
			}
		}
		else if (thisDataMsg->type == LORA_BROADCAST)
		{
			// This is a broadcast. Forward to all direct nodes, but not to the one who sent it
			myLog_d("Handling broadcast with ID %08X from %08X", thisDataMsg->dest, thisDataMsg->from);
			// Check if this broadcast is coming from ourself
			if ((thisDataMsg->dest & 0xFFFFFF00) == (deviceID & 0xFFFFFF00))
			{
				myLog_w("We received our own broadcast, dismissing it");
				return;
			}
			// Check if we handled this broadcast already
			if (isOldBroadcast(thisDataMsg->dest))
			{
				myLog_w("Got an old broadcast, dismissing it");
				return;
			}

			// Put broadcast into send queue
			if (!addSendRequest(thisDataMsg, tempSize))
			{
				myLog_e("Cannot forward broadcast because send queue is full");
			}

			// This is a broadcast, call user callback to handle the data
			myLog_d("Got data broadcast %s", (char *)thisDataMsg->data);
			if ((_MeshEvents != NULL) && (_MeshEvents->DataAvailable != NULL))
			{
				_MeshEvents->DataAvailable(thisDataMsg->from, thisDataMsg->data, tempSize - 12, rxRssi, rxSnr);
			}
		}
	}
	else
	{
		myLog_e("Invalid package");
		for (int idx = 0; idx < tempSize; idx++)
		{
			Serial.printf("%02X ", rxBuffer[idx]);
		}
		Serial.println("");
	}
}

/**
 * Callback after a package was successfully sent
 */
void OnTxDone(void)
{
	myLog_w("LoRa send finished");
	loraState = MESH_IDLE;

	// Restart listening
	Radio.Standby();
	// Radio.Rx(0);
	Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
}

/**
 * Callback if sending a package timed out
 * Not sure what triggers that, it never occured
 */
void OnTxTimeout(void)
{
	myLog_w("LoRa TX timeout");
	loraState = MESH_IDLE;

	// Restart listening
	Radio.Standby();
	// Radio.Rx(0);
	Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
}

/**
 * Callback if sending a package timed out
 * Not sure what triggers that, it never occured
 */
void OnTxTimerTimeout(void)
{
	myLog_w("LoRa TX SW timer timeout");
	loraState = MESH_IDLE;

	// Internal timer timeout, maybe some problem with SX126x ???
	Radio.Standby();
	// Radio.Rx(0);
	Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
}

/**
 * Callback if waiting for a package timed out
 */
void OnRxTimeout(void)
{
	myLog_w("LoRa RX timeout");

	if (loraState != MESH_TX)
	{
		loraState = MESH_IDLE;

		// Restart listening
		Radio.Standby();
		// Radio.Rx(0);
		Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
	}
}

/**
 * Callback if waiting for a package timed out
 */
void OnRxTimerTimeout(void)
{
	myLog_w("LoRa RX SW timer timeout");
	if (loraState != MESH_TX)
	{
		loraState = MESH_IDLE;

		// Internal timer timeout, maybe some problem with SX126x ???
		Radio.Standby();
		// Radio.Rx(0);
		Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
	}
}

/**
 * Callback if waiting for a package failed
 * Not sure what triggers that, it never occured
 */
void OnRxError(void)
{
	myLog_w("LoRa CRC error");
	if (loraState != MESH_TX)
	{
		loraState = MESH_IDLE;

		// Restart listening
		Radio.Standby();
		// Radio.Rx(0);
		Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
	}
}

/**
 * Callback if a preamble is detected during RX
 * That means a package should be coming in
 */
void OnPreAmbDetect(void)
{
	myLog_d("OnPreAmbDetect");
	// Put LoRa modem state into RX as a message is coming in
	loraState = MESH_RX;
	preambTimeout = millis();
}

/**
 * Callback if the Channel Activity Detection has finished
 * Starts sending the ACK package if the channel is available
 * Retries 5 times if the channel was busy.
 * Returns to listen mode if the channel was occupied 5 times
 *
 * @param cadResult
 * 		True if channel activity was detected
 * 		False if the channel is available
 */
void OnCadDone(bool cadResult)
{
	// Not used
	if (cadResult)
	{
		myLog_d("CAD returned channel busy");
		channelFreeRetryNum++;
		if (channelFreeRetryNum >= CAD_RETRY)
		{
			myLog_e("CAD returned channel busy %d times, giving up", CAD_RETRY);
			loraState = MESH_IDLE;
			channelFreeRetryNum = 0;
			// Restart listening
			Radio.Standby();
			// Radio.Rx(0);
			Radio.SetRxDutyCycle(RX_SLEEP_TIMES);
		}
		else
		{
			// Wait a little bit before retrying
			delay(250);
			Radio.Standby();
			Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
			Radio.StartCad();
		}
	}
	else
	{
		myLog_d("CAD returned channel free");
		myLog_d("Sending %d bytes", txLen);
		channelFreeRetryNum = 0;

		// Send the data package
		Radio.Standby();
		Radio.Send((uint8_t *)&txPckg, txLen);
	}
}

/**
 * Add a data package to the queue
 * @param package
 * 			dataPckg * to the package data
 * @param msgSize
 * 			Size of the data package
 * @return result
 * 			TRUE if task could be added to queue
 * 			FALSE if queue is full or not initialized
 */
bool addSendRequest(dataMsg *package, uint8_t msgSize)
{
	if (sendQueue != NULL)
	{
#ifdef ESP32
		portENTER_CRITICAL(&accessMsgQueue);
#else
		taskENTER_CRITICAL();
#endif
		// Find unused entry in queue list
		int next = SEND_QUEUE_SIZE;
		for (int idx = 0; idx < SEND_QUEUE_SIZE; idx++)
		{
			if (sendMsg[idx].type == LORA_INVALID)
			{
				next = idx;
				break;
			}
		}

		if (next != SEND_QUEUE_SIZE)
		{
			// Found an empty entry!
			memcpy(&sendMsg[next], package, msgSize);
			sendMsgSize[next] = msgSize;

			myLog_d("Queued msg #%d with len %d", next, msgSize);

			// Try to add to cloudTaskQueue
			if (xQueueSend(sendQueue, &next, (TickType_t)1000) != pdTRUE)
			{
				myLog_e("Send queue is busy");
#ifdef ESP32
				portEXIT_CRITICAL(&accessMsgQueue);
#else
				taskEXIT_CRITICAL();
#endif
				return false;
			}
			else
			{
				myLog_v("Send request queued:");
#ifdef ESP32
				portEXIT_CRITICAL(&accessMsgQueue);
#else
				taskEXIT_CRITICAL();
#endif
				return true;
			}
		}
		else
		{
			myLog_e("Send queue is full");
			// Queue is already full!
#ifdef ESP32
			portEXIT_CRITICAL(&accessMsgQueue);
#else
			taskEXIT_CRITICAL();
#endif
			return false;
		}
	}
	else
	{
		myLog_e("Send queue not yet initialized");
		return false;
	}
}
