#include "Arduino.h"

struct mapMsg
{
	uint8_t mark1 = 'L';
	uint8_t mark2 = 'o';
	uint8_t mark3 = 'R';
	uint8_t type = 5;
	uint32_t dest = 0;
	uint32_t from = 0;
	uint8_t nodes[48][5];
};

struct dataMsg
{
	uint8_t mark1 = 'L';
	uint8_t mark2 = 'o';
	uint8_t mark3 = 'R';
	uint8_t type = 0;
	uint32_t dest = 0;
	uint32_t from = 0;
	uint32_t orig = 0;
	uint8_t data[243];
};

/**
 * Mesh callback functions
 */
typedef struct
{
	/**
     * Data available callback prototype.
     *
     * @param payload 
	 * 			Received buffer pointer
     * @param size    
	 * 			Received buffer size
     * @param rssi    
	 * 			RSSI value computed while receiving the frame [dBm]
     * @param snr     
	 * 			SNR value computed while receiving the frame [dB]
     *                     FSK : N/A ( set to 0 )
     *                     LoRa: SNR value in dB
     */
	void (*DataAvailable)(uint32_t fromID, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
	
	/**
     * Nodes list change callback prototype.
     */
	void (*NodesListChanged)(void);

} MeshEvents_t;

// LoRa Mesh functions & variables
void initMesh(MeshEvents_t *events, int numOfNodes);
void meshTask(void *pvParameters);
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnPreAmbDetect(void);
void OnCadDone(bool cadResult);
bool addSendRequest(dataMsg *package, uint8_t msgSize);
extern TaskHandle_t meshTaskHandle;
extern volatile xQueueHandle meshMsgQueue;

/** Size of map message buffer without subnode */
#define MAP_HEADER_SIZE 12
/** Size of data message buffer without subnode */
#define DATA_HEADER_SIZE 16

/** Number of retries if CAD shows busy */
#define CAD_RETRY 20



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


// #define RX_SLEEP_TIMES 2 * 32768 * 1000 * 15.625, 10 * 32768 * 1000 * 15.625


//https://www.st.com/content/ccc/resource/training/technical/product_training/group1/ee/e6/ef/5b/7b/43/48/95/STM32WL-Peripheral-LoRaStack_LORASTACK/files/STM32WL-Peripheral-LoRaStack_LORASTACK.pdf/_jcr_content/translations/en.STM32WL-Peripheral-LoRaStack_LORASTACK.pdf
// LoRa definitions
#define RF_FREQUENCY 868370000    // Hz
#define TX_OUTPUT_POWER 14		  // dBm	
#define LORA_SPREADING_FACTOR 11  // [SF7..SF12]
#define LORA_CODINGRATE 1		  // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]	
#define LORA_BANDWIDTH 2		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx	
#define LORA_SYMBOL_TIMEOUT 0   // Symbols	
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 7000				
#define TX_TIMEOUT_VALUE 7000				

struct nodesList
{
	uint32_t nodeId;
	uint32_t firstHop;
	time_t timeStamp;
	uint8_t numHops;
};

bool getRoute(uint32_t id, nodesList *route);
boolean addNode(uint32_t id, uint32_t hop, uint8_t numHops);
void removeNode(uint32_t id);
void clearSubs(uint32_t id);
bool cleanMap(void);
uint8_t nodeMap(uint32_t subs[], uint8_t hops[]);
uint8_t nodeMap(uint8_t nodes[][5]);
uint8_t numOfNodes();
bool getNode(uint8_t nodeNum, uint32_t &nodeId, uint32_t &firstHop, uint8_t &numHops);
uint32_t getNextBroadcastID(void);
bool isOldBroadcast(uint32_t broadcastID);

extern SemaphoreHandle_t accessNodeList;
extern nodesList *nodesMap;
extern int _numOfNodes;
