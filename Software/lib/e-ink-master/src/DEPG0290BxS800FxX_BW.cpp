#include "DEPG0290BxS800FxX_BW.h"

/************************************** init ************************************************/
void DEPG0290BxS800FxX_BW::EPD_Init(void) {
#if defined( ESP32 )
	SPI.begin(this->clk_pin, MISO,MOSI, this->cs_pin);
#elif defined( ESP8266 )
	SPI.pins(this->clk_pin, MISO, MOSI, this->cs_pin);
	SPI.begin();
#elif defined( CubeCell_Board )//AB01
    SPI.begin(this->cs_pin, this->freq, this->spi_num);
#endif

	if (IfInit() != 0) {
		Serial.print("e-Paper init failed");
		return;
	}
	Reset();
    WaitUntilIdle();
    SendCommand(0x12); // soft reset
    WaitUntilIdle();
    Serial.println("e-Paper init OK!");	
}

/****************************** All screen update *******************************************/
void DEPG0290BxS800FxX_BW::EPD_ALL_image(const unsigned char *datas) {
   	unsigned int i;

    SendCommand(0x24);   //write RAM for black(0)/white (1)

	for (int i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        SendData(pgm_read_byte(&datas[i]));
    }	

   	EPD_Update();		 
}

/********************************* update ***************************************************/
void DEPG0290BxS800FxX_BW::EPD_Update(void) {      			
    SendCommand(0x20);
    WaitUntilIdle();
    DelayMs(100);   
}

/********************************** deep sleep **********************************************/
void DEPG0290BxS800FxX_BW::EPD_DeepSleep(void) {  	
  SendCommand(0x10); //enter deep sleep
  SendData(0x01); 
}

/********************************* Display All Black ****************************************/
void DEPG0290BxS800FxX_BW::EPD_WhiteScreen_Black(void) {
    EPD_Load_Data(0x00);
}

/********************************* Display All White ****************************************/
void DEPG0290BxS800FxX_BW::EPD_WhiteScreen_White(void) {
    EPD_Load_Data(0XFF);
}

/********************************** Load Data ***********************************************/
void DEPG0290BxS800FxX_BW::EPD_Load_Data(unsigned char data) {
    unsigned int i,k;
    SendCommand(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<296;k++) {
        for(i=0;i<16;i++) {
            SendData(data);
        }
    }
	
	EPD_Update();
}

/* DEPG0290BxS800FxX_BW.cpp END */