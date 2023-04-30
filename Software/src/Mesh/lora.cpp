#include "main.h"

/** HW configuration structure for the LoRa library */
extern hw_config _hwConfig;

#ifdef ESP32
#ifdef IS_WROVER
// ESP32 Wrover - SX126x pin configuration
/** LORA RESET */
int PIN_LORA_RESET = 16;
/** LORA DIO_1 */
int PIN_LORA_DIO_1 = 4;				
/** LORA SPI BUSY */
int PIN_LORA_BUSY = 17;
/** LORA SPI CS */
int PIN_LORA_NSS = 5;
/** LORA SPI CLK */
int PIN_LORA_SCLK = 18;
/** LORA SPI MISO */
int PIN_LORA_MISO = 19;
/** LORA SPI MOSI */
int PIN_LORA_MOSI = 23;
/** LORA ANTENNA TX ENABLE */ 26
int RADIO_TXEN = -1;
/** LORA ANTENNA RX ENABLE */ 25
int RADIO_RXEN = -1;

#define LED_BUILTIN 15
#else
// ESP32 Feather - SX126x pin configuration
/** LORA RESET */
int PIN_LORA_RESET = 16;
/** LORA DIO_1 */
int PIN_LORA_DIO_1 = 4;				
/** LORA SPI BUSY */
int PIN_LORA_BUSY = 17;
/** LORA SPI CS */
int PIN_LORA_NSS = 5;
/** LORA SPI CLK */
int PIN_LORA_SCLK = 18;
/** LORA SPI MISO */
int PIN_LORA_MISO = 19;
/** LORA SPI MOSI */
int PIN_LORA_MOSI = 23;
/** LORA ANTENNA TX ENABLE */	
int RADIO_TXEN = -1;
/** LORA ANTENNA RX ENABLE */	
int RADIO_RXEN = -1;							
#endif
#ifdef RED_ESP
#undef LED_BUILTIN
#define LED_BUILTIN 16
#endif
#endif


/** Callback if data was received over LoRa SX1262 */
void OnLoraData(uint32_t fromID, uint8_t *rxPayload, uint16_t rxSize, int16_t rxRssi, int8_t rxSnr);
/** Callback if Mesh map changed */
void onNodesListChange(void);

/** Structure with Mesh event callbacks */
static MeshEvents_t MeshEvents;

/**
 * Initialize the LoRa HW
 * @return bool
 * 		True if initialization was successfull, false if not
 */
bool initLoRa(void)
{
	bool initResult = true;
#if defined(_VARIANT_RAK4630_) // WisBlock RAK4631
	if (lora_rak4630_init() != 0)
	{
		myLog_e("Error in hardware init");
		initResult = false;
	}
#elif defined(ESP32) || defined(ADAFRUIT)
	// Define the HW configuration between MCU and SX126x
	_hwConfig.CHIP_TYPE = SX1262_CHIP;		   // eByte E22 module with an SX1262
	_hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
	_hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;	 // LORA SPI CS
	_hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
	_hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
	_hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
	_hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
	_hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
	_hwConfig.RADIO_TXEN = RADIO_TXEN;		   // LORA ANTENNA TX ENABLE
	_hwConfig.RADIO_RXEN = RADIO_RXEN;		   // LORA ANTENNA RX ENABLE
	_hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an eByte E22 module which uses RXEN and TXEN pins as antenna control
	_hwConfig.USE_DIO3_TCXO = true;			   // Example uses an eByte E22 module which uses DIO3 to control oscillator voltage
	_hwConfig.USE_DIO3_ANT_SWITCH = false;	 // Only Insight ISP4520 module uses DIO3 as antenna control

	if (lora_hardware_init(_hwConfig) != 0)
	{
		myLog_e("Error in hardware init");
	}
#else // ISP4520
	if (lora_isp4520_init(SX1262) != 0)
	{
		myLog_e("Error in hardware init");
		initResult = false;
	}
#endif
	uint16_t readSyncWord = 0;
	SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t *)&readSyncWord, 2);
	myLog_w("Got syncword %X", readSyncWord);							

	MeshEvents.DataAvailable = OnLoraData;
	MeshEvents.NodesListChanged = onNodesListChange;

	// Initialize the LoRa Mesh
	// * events, number of nodes, frequency, TX power
#ifdef ESP32
	initMesh(&MeshEvents, 48);
#else
	initMesh(&MeshEvents, 30);
#endif
	return initResult;
}