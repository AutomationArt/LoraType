/**
 *  @filename   :   e_ink.cpp
 *  @brief      :   Implements for e-ink library
 *  @author     :   HelTec AutoMation
 *
 *  
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include "e_ink.h"
#include "lut.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes LOW
 */
void Epd::WaitUntilIdle(void) {
    while(DigitalRead(busy_pin) == HIGH) {      //LOW: idle, HIGH: busy
        DelayMs(100);
    }      
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    DigitalWrite(reset_pin, LOW);                //module reset    
    DelayMs(200);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);    
}

#if SPECIAL_SCREEN

int Epd::Init(const unsigned char* lut) 
{
    /* this calls the peripheral hardware interface, see epdif */
#if defined( ESP32 )
	SPI.begin(CLK_PIN,MISO,SDI_PIN,CS_PIN);
#elif defined( ESP8266 )
	SPI.pins(CLK_PIN,MISO,SDI_PIN,CS_PIN);
	SPI.begin();
#endif
    if (IfInit() != 0) {
        Serial.print("e-Paper init failed");
		return -1;
    }
#if defined ( USE_154_BW_GREEN )
/* EPD hardware init start */
    this->lut = lut;
    //Reset();
    // WaitUntilIdle();
    // SendCommand(SW_RESET);
    // WaitUntilIdle();
    SendCommand(DRIVER_OUTPUT_CONTROL);
    SendData((EPD_HEIGHT - 1) & 0xFF);
    SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    SendCommand(DATA_ENTRY_MODE_SETTING); //data entry mode       
    SendData(0x01);
    SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION); //set Ram-X address start/end position   
    SendData(0x00);
    SendData(0x18);    //0x0C-->(18+1)*8=200
    SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION); //set Ram-Y address start/end position          
    SendData(0xC7);   //0xC7-->(199+1)=200
    SendData(0x00);
    SendData(0x00);
    SendData(0x00); 
    SendCommand(BORDER_WAVEFORM_CONTROL); //BorderWavefrom
    SendData(0x01);	  
    SendCommand(0x18); 
    SendData(0x80);	
    SendCommand(DISPLAY_UPDATE_CONTROL_2); // //Load Temperature and waveform setting.
    SendData(0XB1);	
    SendCommand(MASTER_ACTIVATION); 
    SendCommand(SET_RAM_X_ADDRESS_COUNTER);   // set RAM x address count to 0;
    SendData(0x00);
    SendCommand(SET_RAM_Y_ADDRESS_COUNTER);   // set RAM y address count to 0X199;    
    SendData(0xC7);
    SendData(0x00);
    // WaitUntilIdle();
    SetLut(this->lut);
    /* EPD hardware init end */
#elif defined ( USE_154_BW_BLUE )
    /* EPD hardware init start */
    this->lut = lut;
    Reset();
    SendCommand(DRIVER_OUTPUT_CONTROL);
    SendData((EPD_HEIGHT - 1) & 0xFF);
    SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    SendCommand(BOOSTER_SOFT_START_CONTROL);
    SendData(0xD7);
    SendData(0xD6);
    SendData(0x9D);
    SendCommand(WRITE_VCOM_REGISTER);
    SendData(0xA8);                     // VCOM 7C
    SendCommand(SET_DUMMY_LINE_PERIOD);
    SendData(0x1A);                     // 4 dummy lines per gate
    SendCommand(SET_GATE_TIME);
    SendData(0x08);                     // 2us per line
    SendCommand(DATA_ENTRY_MODE_SETTING);
    SendData(0x03);                     // X increment; Y increment
    SetLut(this->lut);
    /* EPD hardware init end */
#elif defined(  USE_290_BW  )
    /* EPD hardware init start */
    this->lut = lut;
    Reset();
    SendCommand(DRIVER_OUTPUT_CONTROL);  //0X01
    SendData((EPD_HEIGHT - 1) & 0xFF);
    SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
    SendData(0x00);                     // GD = 0; SM = 0; TB = 0;
    SendCommand(BOOSTER_SOFT_START_CONTROL); //0X0C
    SendData(0xD7);
    SendData(0xD6);
    SendData(0x9D);
    SendCommand(WRITE_VCOM_REGISTER);   //0X2C
    SendData(0xA8);  //9A                   // VCOM 7C
    SendCommand(SET_DUMMY_LINE_PERIOD); //0X3A
    SendData(0x1A);                     // 4 dummy lines per gate
    SendCommand(SET_GATE_TIME);         //0X3B
    SendData(0x08);                     // 2us per line
    SendCommand(DATA_ENTRY_MODE_SETTING); //0X11
    SendData(0x03);  //SendData(0x03);
    SetLut(this->lut);
	/* EPD hardware init end */
#endif
	Serial.println("e-Paper init OK");
    return 0;
}


/**
 *  @brief: set the look-up table register
 */
void Epd::SetLut(const unsigned char* lut) {
    this->lut = lut;
    SendCommand(WRITE_LUT_REGISTER);
    /* the length of look-up table is 30 bytes */
    for (int i = 0; i < 30; i++) {
        SendData(this->lut[i]);
    }
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 */
void Epd::SetFrameMemory(
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
    if (x + image_width >= this->width) {
        x_end = this->width - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= this->height) {
        y_end = this->height - 1;
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
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 *
 *          Question: When do you use this function instead of 
 *          void SetFrameMemory(
 *              const unsigned char* image_buffer,
 *              int x,
 *              int y,
 *              int image_width,
 *              int image_height
 *          );
 *          Answer: SetFrameMemory with parameters only reads image data
 *          from the RAM but not from the flash in AVR chips (for AVR chips,
 *          you have to use the function pgm_read_byte to read buffers 
 *          from the flash).
 */
void Epd::SetFrameMemory(const unsigned char* image_buffer) {
#ifndef USE_154_BW_GREEN
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
#endif
    SendCommand(WRITE_RAM);
    /* send the image data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(pgm_read_byte(&image_buffer[i]));
    }
}


/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void Epd::ClearFrameMemory(unsigned char color) {
#ifdef USE_154_BW_GREEN
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
#endif
    SendCommand(WRITE_RAM);
    /* send the color data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
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
void Epd::DisplayFrame(void) {
    SendCommand(DISPLAY_UPDATE_CONTROL_2);
#if defined (USE_154_BW_GREEN)
    SendData(0xf7);
#elif defined ( USE_290_BW ) || defined (USE_154_BW_BLUE)
    SendData(0xc4);
#endif
    SendCommand(MASTER_ACTIVATION);
    SendCommand(TERMINATE_FRAME_READ_WRITE);
#ifndef  USE_290_BW
    WaitUntilIdle();
#endif
}

/**
 *  @brief: private function to specify the memory area for data R/W
 */
void Epd::SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
    SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
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
void Epd::SetMemoryPointer(int x, int y) {
    SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x >> 3) & 0xFF);
    SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    SendData(y & 0xFF);
    SendData((y >> 8) & 0xFF);
#if defined ( USE_154_BW_BLUE )
    WaitUntilIdle();
#endif
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          You can use Epd::Init() to awaken
 */
void Epd::Sleep() {
    SendCommand(DEEP_SLEEP_MODE);
    WaitUntilIdle();
}


#elif ORDINARY_SCREEN

int Epd::Init(const unsigned char *lut)
{
	 /* this calls the peripheral hardware interface, see epdif */
	//Serial.begin(115200);
#if defined( ESP32 )
	SPI.begin(CLK_PIN,MISO,SDI_PIN,CS_PIN);

#elif defined( ESP8266 )
	SPI.pins(CLK_PIN,MISO,SDI_PIN,CS_PIN);
	SPI.begin();
#endif
	if (IfInit() != 0) {
	    Serial.print("e-Paper init failed!");
        return -1;
    }
#if  USE_154
/* EPD hardware init start */
    //Reset();
    SendCommand(POWER_SETTING);
    SendData(0x07);
    SendData(0x00);
    SendData(0x08);
    SendData(0x00);
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);
    SendCommand(POWER_ON);
    WaitUntilIdle();
    SendCommand(PANEL_SETTING);
    SendData(0x0f);
    SendData(0x0d);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0xf7);
//  SendCommand(PLL_CONTROL);
//  SendData(0x39);
    SendCommand(RESOLUTION_SETTING);
    SendData(0x98);
    SendData(0x00);
    SendData(0x98);
    SendCommand(VCM_DC_SETTING);
    SendData(0xf7);
    SetLutBw();
    SetLutRed();
    /* EPD hardware init end */
#elif USE_213
    /* EPD hardware init start */
    Reset();
    SendCommand(POWER_SETTING);
    SendData(0x03);
    SendData(0x00);
    SendData(0x2b);
    SendData(0x2b);
    SendData(0x03);
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);
    SendCommand(POWER_ON);
    // WaitUntilIdle();
    SendCommand(PANEL_SETTING);
#if defined ( USE_213_BWR ) || defined (  USE_213_BWY  )
    SendData(0x8F);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x37);
    SendCommand(RESOLUTION_SETTING);
    SendData(0x68);     // width: 104
    SendData(0x00);
    SendData(0xD4);     // height: 212
#elif defined ( USE_213_BW ) || defined ( USE_213_BWSoft )
    SendData(0xbf);		//LUT from OTP￡?128x296
    SendData(0x0d);		//VCOM to 0V fast
    SendCommand(PLL_CONTROL);			//PLL setting
    SendData (0x3a);   // 3a 100HZ   29 150Hz 39 200HZ	31 171HZ
    SendCommand(RESOLUTION_SETTING);			//resolution setting
    SendData (0x68);
    SendData (0x00);
    SendData (0xd4);
    SendCommand(VCM_DC_SETTING);			//vcom_DC setting  	
    SendData (0x28);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);			//VCOM AND DATA INTERVAL SETTING			
    SendData(0x97);		//WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
    SetLut();
#endif
    /* EPD hardware init end */
#elif USE_260
	 /** EPD2IN60B Initialization **/
    /* EPD hardware init start */
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);
    SendCommand(POWER_ON);
    SendCommand(PANEL_SETTING);
#if defined(  USE_260_BW  )
    SendData(0xbf);
    SendData(0x0d);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x97);
    SendCommand(RESOLUTION_SETTING);
    SendData(0x98);     // width: 104
    SendData(0x01);
    SendData(0x28);     // height: 212
    SetLut();
#elif defined (  USE_260_BWR  )
    SendData(0x0f);
	SendData(0x0d);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
	SendData(0x77);
	SendCommand(RESOLUTION_SETTING);
	SendData(0x98);     // width: 104
	SendData(0x01);
	SendData(0x28);     // height: 212
#endif
    /* EPD hardware init end */
#elif USE_270
	/** EPD2IN70B Initialization **/
    /* EPD hardware init start */
#if defined ( USE_270_BW )
    Reset();
    SendCommand(POWER_SETTING);
    SendData(0x03);                  // VDS_EN, VDG_EN
    SendData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    SendData(0x2b);                  // VDH
    SendData(0x2b);                  // VDL
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x07);
    SendData(0x07);
    SendData(0x17);
    SendCommand(PARTIAL_DISPLAY_REFRESH);
    SendData(0x00);
    SendCommand(POWER_ON);
    // WaitUntilIdle();
    SendCommand(PANEL_SETTING);
    SendData(0xBF);        //KW-BF   KWR-AF    BWROTP 0f
    SendCommand(PLL_CONTROL);
    SendData(0x3A);       //3A 100HZ   29 150Hz 39 200HZ    31 171HZ
    SendCommand(0x61);
    SendData(0x00);
    SendData(0xb0);
    SendData(0x01);
    SendData(0x08);
    SendCommand(VCM_DC_SETTING);
    SendData(0x08);
    DelayMs(2);
    SendCommand(0X50);			//VCOM AND DATA INTERVAL SETTING			
    SendData(0x97);		//WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
    SetLut();
#elif defined ( USE_270_BWR )
    Reset();
	SendCommand(BOOSTER_SOFT_START);  // BOOSTER_SOFT_START
	SendData(0x07);
	SendData(0x07);
	SendData(0x17);
    SendCommand(PANEL_SETTING); // PANEL_SETTING
	SendData(0x4f); // KW-BF   KWR-AF BWROTP 0f
	SendCommand(PARTIAL_DISPLAY_REFRESH); // PARTIAL_DISPLAY_REFRESH
	SendData(0x00);
    SendCommand(POWER_ON); // POWER_ON
	//WaitUntilIdle();
	
#endif
    /* EPD hardware init end */
#elif USE_290
    /* EPD hardware init start */
    Reset();
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);
    SendCommand(POWER_ON);
    //WaitUntilIdle();
    SendCommand(PANEL_SETTING);
    SendData(0x8F);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x77);
    SendCommand(RESOLUTION_SETTING);
    SendData(0x80);
    SendData(0x01);
    SendData(0x28);
    SendCommand(VCM_DC_SETTING);
    SendData(0X0A);
    /* EPD hardware init end */
#elif USE_420
    /* EPD hardware init start */
#if defined ( USE_420_BWR ) || defined ( USE_420_BWY )
    Reset();
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);     //07 0f 17 1f 27 2F 37 2f
    SendCommand(POWER_ON);
    WaitUntilIdle();
    SendCommand(PANEL_SETTING);
    SendData(0x0F);     // LUT from OTP
#elif defined ( USE_420_BW )
	/** EPD4IN20B Initialization **/
	/* EPD hardware init start */
    Reset();
    SendCommand(POWER_SETTING);
    SendData(0x03);                  // VDS_EN, VDG_EN
    SendData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    SendData(0x2b);                  // VDH
    SendData(0x2b);                  // VDL
    SendData(0xff);                  // VDHR
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);                  //07 0f 17 1f 27 2F 37 2f
    SendCommand(POWER_ON);
    WaitUntilIdle();
    SendCommand(PANEL_SETTING);
    SendData(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
    SendData(0x0b);
    SendCommand(PLL_CONTROL);
    SendData(0x3c);        // 3A 100HZ   29 150Hz 39 200HZ  31 171HZ
#endif
    /* EPD hardware init end */
#elif USE_583
  /** EPD5IN83B Initialization **/
	/* EPD hardware init start */
    SendCommand(BOOSTER_SOFT_START);
	SendData(0xC7);
	SendData(0xCC);
	SendData(0x28);
	SendCommand(POWER_SETTING);
	SendData(0x37);
	SendData(0x00);
	SendCommand(POWER_ON);
	//WaitUntilIdle();
	SendCommand(PANEL_SETTING);
	SendData(0xcf);//RES 11b: 600×448  LUT_EN from register 0
	SendData(0x08);
	SendCommand(RESOLUTION_SETTING);
	SendData(0x02);     // width: 600
	SendData(0x58);
	SendData(0x01);     // height: 448
	SendData(0xC0);
	SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
	SendData(0x77);// Floating
	SendCommand(0xe5);
	SendData(0x03);
	
    SendCommand(TCON_SETTING);
	SendData(0x22);
	SendCommand(0xe5);           //FLASH MODE
	SendData(0x03);
	
	SendCommand(PLL_CONTROL);//控制刷新速度
#if defined ( USE_583_BW )
	SendData(0x3c);
	SendCommand(VCM_DC_SETTING);
	SendData(0x1e);
#else //defined ( USE_583_BWY ) || defined ( USE_583_BWR )
	SendData(0x3a);
	SendCommand(VCM_DC_SETTING);
	SendData(0x28);
#endif
    /* EPD hardware init end */
#elif USE_750
	/** EPD7IN50B Initialization **/
	/* EPD hardware init start */
	SendCommand(BOOSTER_SOFT_START);
	SendData(0xC7);
	SendData(0xCC);
	SendData(0x28);
	SendCommand(POWER_SETTING);
	SendData(0x37);
	SendData(0x00);
	SendCommand(POWER_ON);
	//WaitUntilIdle();
	SendCommand(PANEL_SETTING);
	SendData(0xcf);//RES 11b: 600×448  LUT_EN from register 0
	SendData(0x08);
	SendCommand(PLL_CONTROL);
	SendData(0x3a);
	SendCommand(RESOLUTION_SETTING);
	SendData(0x02);     // width: 600
	SendData(0x80);
	SendData(0x01);     // height: 448
	SendData(0x80);
	SendCommand(VCM_DC_SETTING);
	SendData(0x28);
	SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
	SendData(0x77);// Floating
	SendCommand(0xe5);
	SendData(0x03);
	/* EPD hardware init end */
#endif
	Serial.print("e-Paper init OK");
    return 0;

}

#if USE_154
/**
 *  @brief: set the look-up tables
 */
void Epd::SetLutBw(void) {
    unsigned int count;
    SendCommand(VCOM_LUT);         //g vcom
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_vcom0[count]));
    }
    SendCommand(W2W_LUT);        //g ww --
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_w[count]));
    }
    SendCommand(B2W_LUT);         //g bw r
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_b[count]));
    }
    SendCommand(W2B_LUT);         //g wb w
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_g1[count]));
    }
    SendCommand(B2B_LUT);         //g bb b
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_g2[count]));
    }
}

void Epd::SetLutRed(void) {
    unsigned int count;
    SendCommand(LUT_RED_0);
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_vcom1[count]));
    }
    SendCommand(LUT_RED_1);
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_red0[count]));
    }
    SendCommand(LUT_RED_2);
    for(count = 0; count < 15; count++) {
        SendData(pgm_read_byte(&lut_red1[count]));
    }
}
#elif defined ( USE_260_BW ) || defined ( USE_213_BW ) || defined ( USE_213_BWSoft )
//LUT download
void Epd :: SetLut(void)
{
    unsigned int count;
    SendCommand(VCOM_LUT);
    for(count=0;count<44;count++)
        {SendData(pgm_read_byte(&lut_vcomDC[count]));}

    SendCommand(W2W_LUT);
    for(count=0;count<42;count++)
        {SendData(pgm_read_byte(&lut_ww[count]));}

    SendCommand(B2W_LUT);
    for(count=0;count<42;count++)
        {SendData(pgm_read_byte(&lut_bw[count]));}

    SendCommand(W2B_LUT);
    for(count=0;count<42;count++)
        {SendData(pgm_read_byte(&lut_wb[count]));}

    SendCommand(B2B_LUT);
    for(count=0;count<42;count++)
        {SendData(pgm_read_byte(&lut_bb[count]));}
}
#elif defined ( USE_270_BW )
void Epd::SetLut(void) {
    unsigned int count;
    SendCommand(VCOM_LUT);                            //vcom
    for(count = 0; count < 44; count++) {
        SendData(pgm_read_byte(&lut_vcom_dc[count]));
    }

    SendCommand(W2W_LUT);                      //ww --
    for(count = 0; count < 42; count++) {
        SendData(pgm_read_byte(&lut_ww[count]));
    }

    SendCommand(B2W_LUT);                      //bw r
    for(count = 0; count < 42; count++) {
        SendData(pgm_read_byte(&lut_bw[count]));
    }

    SendCommand(W2B_LUT);                      //wb w
    for(count = 0; count < 42; count++) {
        SendData(pgm_read_byte(&lut_bb[count]));
    }

    SendCommand(B2B_LUT);                      //bb b
    for(count = 0; count < 42; count++) {
        SendData(pgm_read_byte(&lut_wb[count]));
    }
}

#endif
/**
 *  @brief: transmit partial data to the SRAM
 */
void Epd::SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {
    SendCommand(PARTIAL_IN);
    SendCommand(PARTIAL_WINDOW);
    SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    SendData(((x & 0xf8) + w  - 1) | 0x07);
    SendData(y >> 8);        
    SendData(y & 0xff);
    SendData((y + l - 1) >> 8);        
    SendData((y + l - 1) & 0xff);
    SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer_black[i]);  
        }  
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);  
        }  
    }
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer_red[i]);  
        }  
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);  
        }  
    }
    DelayMs(2);
    SendCommand(PARTIAL_OUT);  
}

/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void Epd::SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l) {
    SendCommand(PARTIAL_IN);
    SendCommand(PARTIAL_WINDOW);
    SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    SendData(((x & 0xf8) + w  - 1) | 0x07);
    SendData(y >> 8);        
    SendData(y & 0xff);
    SendData((y + l - 1) >> 8);        
    SendData((y + l - 1) & 0xff);
    SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer_black[i]);  
        }  
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);  
        }  
    }
    DelayMs(2);
    SendCommand(PARTIAL_OUT);  
}

/**
 *  @brief: transmit partial data to the red part of SRAM
 */
void Epd::SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l) {
    SendCommand(PARTIAL_IN);
    SendCommand(PARTIAL_WINDOW);
    SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    SendData(((x & 0xf8) + w  - 1) | 0x07);
    SendData(y >> 8);        
    SendData(y & 0xff);
    SendData((y + l - 1) >> 8);        
    SendData((y + l - 1) & 0xff);
    SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer_red[i]);  
        }  
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);  
        }  
    }
    DelayMs(2);
    SendCommand(PARTIAL_OUT);  
}

/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red) {
    if (frame_buffer_black != NULL) {
        SendCommand(DATA_START_TRANSMISSION_1);
        DelayMs(2);
        for (int i = 0; i < this->width * this->height / 8; i++) {
            SendData(pgm_read_byte(&frame_buffer_black[i]));
        }
        DelayMs(2);
    }
    if (frame_buffer_red != NULL) {
        SendCommand(DATA_START_TRANSMISSION_2);
        DelayMs(2);
        for (int i = 0; i < this->width * this->height / 8; i++) {
            SendData(pgm_read_byte(&frame_buffer_red[i]));
        }
        DelayMs(2);
    }
    SendCommand(DISPLAY_REFRESH);
   // WaitUntilIdle();
}
#if defined ( USE_270_BW ) || defined ( USE_420_BW )|| \
    defined ( USE_290_BWSoft ) || defined ( USE_213_BWSoft ) || \
    defined ( USE_213_BW ) || defined ( USE_260_BW )
/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char* frame_buffer_black)
{
	if (frame_buffer_black != NULL)
	{
		#if defined ( USE_270_BW ) || defined ( USE_213_BW ) || defined ( USE_213_BWSoft ) ||\
            defined ( USE_260_BW )
			SendCommand(DATA_START_TRANSMISSION_1);
			DelayMs(2);
			for (int i = 0; i < this->width * this->height / 8; i++)
			{
				SendData(0x00);
			}
			DelayMs(2);
		#endif
		SendCommand(DATA_START_TRANSMISSION_2);
		DelayMs(2);
		for (int i = 0; i < this->width * this->height / 8; i++)
		{
			SendData(pgm_read_byte(&frame_buffer_black[i]));
		}
		DelayMs(2);
		SendCommand(DISPLAY_REFRESH);
	}
}
#elif defined ( USE_583_BW ) || defined ( USE_583_THREE_COLORS ) || \
	  defined ( USE_750_BW ) || defined ( USE_750_BWR ) || defined ( USE_750_BWY )

#if defined ( USE_583_THREE_COLORS ) ||  defined ( USE_750_BWR ) || defined ( USE_750_BWY )
/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char *frame_buffer)
{
	if (frame_buffer != NULL)
	{
		SendCommand(DATA_START_TRANSMISSION_1);
		DelayMs(2);
		unsigned long int i;
		unsigned char j, temp1, temp2, temp3;

		for (i = 0; i < COUNTER; i++)
		{
			temp1 = pgm_read_byte(&frame_buffer[i]);
			for (j = 0; j < 2; j++)         //2bit to 4bit
			{
				temp2 = temp1 & 0xc0;   //Analysis the first 2bit
				if (temp2 == 0xc0)
					temp3 = 0x00; 			 //black(2bit to 4bit)
				else if (temp2 == 0x00)
					temp3 = 0x03;        //white(2bit to 4bit)
				else
					temp3 = 0x04;        //red(2bit to 4bit)

				temp3 <<= 4;            //move to the Hight 4bit
				temp1 <<= 2;            //move 2bit
				temp2 = temp1 & 0xc0;   //Analysis the second 2bit
				if (temp2 == 0xc0)
					temp3 |= 0x00;       //black(2bit to 4bit),Data consolidation
				else if (temp2 == 0x00)
					temp3 |= 0x03;       //white(2bit to 4bit),Data consolidation
				else
					temp3 |= 0x04;       //red(2bit to 4bit),Data consolidation

				temp1 <<= 2;
				SendData(temp3);
			}
		}
		DelayMs(2);
	}
	SendCommand(DISPLAY_REFRESH);
	//DisplayFrame();
	//WaitUntilIdle();
}
#elif defined ( USE_583_BW ) ||  defined ( USE_750_BW )
/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char *frame_buffer)
{
	if (frame_buffer != NULL)
	{
		SendCommand(DATA_START_TRANSMISSION_1);
		DelayMs(2);
		unsigned long int i;
		unsigned char j, temp1, temp2;

		for (i = 0; i < 33600; i++)
		{
			temp1 = pgm_read_byte(&frame_buffer[i]);
			for (j = 0; j < 8; j++)
			{
				if (temp1 & 0x80)
					temp2 = 0x00;
				else
					temp2 = 0x03;

				temp2 <<= 4;
				temp1 <<= 1;
				j++;

				if (temp1 & 0x80)
					temp2 |= 0x00;
				else
					temp2 |= 0x03;

				temp1 <<= 1;

				SendData(temp2);

			}
		}
		DelayMs(2);
	}
	SendCommand(DISPLAY_REFRESH);
	//DisplayFrame();
	//WaitUntilIdle();
}

#endif

#endif
/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void Epd::ClearFrame(void) {
#ifndef USE_583_THREE_COLORS
    SendCommand(DATA_START_TRANSMISSION_1);           
    DelayMs(2);
    for(int i = 0; i < width * height / 8; i++) {
        SendData(0xFF);  
    }  
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);           
    DelayMs(2);
    for(int i = 0; i < width * height / 8; i++) {
        SendData(0xFF);  
    }  
    DelayMs(2);
#endif
}

/**
 * @brief: This displays the frame data from SRAM
 */
void Epd::DisplayFrame(void) {
    SendCommand(DISPLAY_REFRESH); 
   // WaitUntilIdle();
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd::Sleep() {
    SendCommand(POWER_OFF);
    WaitUntilIdle();
    SendCommand(DEEP_SLEEP);
    SendData(0xA5);     // check code
}
//#endif

#endif

Epd epd;
/* END OF FILE */


