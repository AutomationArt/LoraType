#include "DEPG0213Bx800FxX_BW.h"

#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF
/************************************** init ************************************************/
void DEPG0213Bx800FxX_BW::EPD_Init(void) {
    /* this calls the peripheral hardware interface, see epdif */
#if defined( ESP32 )
	SPI.begin(this->clk_pin,MISO,MOSI, this->cs_pin);

#elif defined( ESP8266 )
	SPI.pins(this->clk_pin,MISO,MOSI, this->cs_pin);
	SPI.begin();
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
void DEPG0213Bx800FxX_BW::EPD_ALL_image(const unsigned char *datas) {
   	unsigned int i;

    // SendCommand(0x4E);     
    // SendData(0x00);

    // SendCommand(0x4F);      
    // SendData(0xf9);
    // SendData(0x00);
    // WaitUntilIdle();	
    SendCommand(0x24);   //write RAM for black(0)/white (1)

	for (int i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        SendData(pgm_read_byte(&datas[i]));
    }	

   	EPD_Update();		 
}

/********************************* update ***************************************************/
void DEPG0213Bx800FxX_BW::EPD_Update(void) {     			
    SendCommand(0x20);
    WaitUntilIdle();
    // DelayMs(100);   
}

/********************************** deep sleep **********************************************/
void DEPG0213Bx800FxX_BW::EPD_DeepSleep(void) {  	
    SendCommand(0x10); //enter deep sleep
    SendData(0x01); 
    DelayMs(100); 	
}

/********************************* Display All Black ****************************************/
void DEPG0213Bx800FxX_BW::EPD_WhiteScreen_Black(void) {
    EPD_Load_Data(0x00);
}

/********************************* Display All White ****************************************/
void DEPG0213Bx800FxX_BW::EPD_WhiteScreen_White(void) {
    EPD_Load_Data(0xff);
}

/********************************** Load Data ***********************************************/
void DEPG0213Bx800FxX_BW::EPD_Load_Data(unsigned char data) {
   unsigned int i,k;
    SendCommand(0x4E);     
    SendData(0x00);

    SendCommand(0x4F);       
    SendData(0xf9);
    SendData(0x00);

    WaitUntilIdle();
    SendCommand(0x24);   //write RAM for black(0)/white (1)

    for(k=0;k<250;k++) {
        for(i=0;i<16;i++) {
            SendData(data);
        }
    }
    EPD_Update();
}

/* DEPG0213Bx800FxX_BW END */



/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 */
void DEPG0213Bx800FxX_BW::SetFrameMemory(
    const unsigned char* image_buffer,
    int x,
    int y,
    int image_width,
    int image_height
) {
    int x_end;
    int y_end;

    if (
        image_buffer == NULL ||
        x < 0 || image_width < 0 ||
        y < 0 || image_height < 0
    ) {
        return;
    }
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= EPD_WIDTH_213) {
        x_end = EPD_WIDTH_213 - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= EPD_HEIGHT_213) {
        y_end = EPD_HEIGHT_213 - 1;
    } else {
        y_end = y + image_height - 1;
    }
    SetMemoryArea(x, y, x_end, y_end);
    SetMemoryPointer(x, y);
    SendCommand(WRITE_RAM);
    /* send the image data */
    for (int j = 0; j < y_end - y + 1; j++) {
        for (int i = 0; i < (x_end - x + 1) / 8; i++) {
            SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
}



/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void DEPG0213Bx800FxX_BW::ClearFrameMemory(unsigned char color) {

    SendCommand(WRITE_RAM);
    /* send the color data */
    for (int i = 0; i < EPD_WIDTH_213 / 8 * EPD_HEIGHT_213; i++) {
        SendData(color);
    }
}
/**
 *  @brief: update the display
 *          there are 2 memory areas embedded in the e-paper display
 *          but once this function is called,
 *          the the next action of SetFrameMemory or ClearFrame will 
 *          set the other memory area.
 */
void DEPG0213Bx800FxX_BW::HalLcd_Partial_Update(void) {
    SendCommand(DISPLAY_UPDATE_CONTROL_2);

    SendData(0xc4);

    SendCommand(MASTER_ACTIVATION);
    SendCommand(TERMINATE_FRAME_READ_WRITE);

    WaitUntilIdle();

}

/**
 *  @brief: private function to specify the memory area for data R/W
 */

void DEPG0213Bx800FxX_BW::SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
    SendCommand(0x11); //data entry mode
    SendData(0x03);
    SendCommand(0x44);
    // SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x_start >> 3) & 0xFF);
    SendData((x_end >> 3) & 0xFF);
    SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    SendData(y_start & 0xFF);
    SendData((y_start >> 8) & 0xFF);
    SendData(y_end & 0xFF);
    SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void DEPG0213Bx800FxX_BW::SetMemoryPointer(int x, int y) {
    SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x >> 3) & 0xFF);
    SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    SendData(y & 0xFF);
    SendData((y >> 8) & 0xFF);

}





void DEPG0213Bx800FxX_BW::Clear(void)
{
    int w, h;
    w = (EPD_WIDTH_213 % 8 == 0)? (EPD_WIDTH_213 / 8 ): (EPD_WIDTH_213 / 8 + 1);
    h = EPD_HEIGHT_213;
 
    SendCommand(0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xff);
        }
    }
    //DISPLAY REFRESH
    EPD_Update();
}
//////////////////////////////////////////////////////////////////////////////////////
