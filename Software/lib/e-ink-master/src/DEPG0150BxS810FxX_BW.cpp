#include "DEPG0150BxS810FxX_BW.h"

/******************************* init ***************************************************/

int DEPG0150BxS810FxX_BW::EPD_Init(void) {
/* this calls the peripheral hardware interface, see epdif */
#if defined( ESP32 )
	SPI.begin(this->clk_pin, 12,13, this->cs_pin);	
#elif defined( ESP8266 )
	SPI.pins(this->clk_pin, MISO, MOSI, this->cs_pin);
	SPI.begin();
#elif defined( CubeCell_Board )//AB01
    SPI.begin(this->cs_pin, this->freq, this->spi_num);
#endif

	if (!this->spiFlag && IfInit() != 0) {
		return -1;
	}
	this->spiFlag = 1;
  Reset();
  WaitUntilIdle();//读busy信号
  SendCommand(0x12); // 软件复位    soft  reset
  WaitUntilIdle();
  
  Serial.println("e-Paper init OK!");	
}


/********************************* update ************************************************/
void DEPG0150BxS810FxX_BW::EPD_Update(void) {     			
  SendCommand(0x20);
  WaitUntilIdle();
}

/*********************************** deep sleep ******************************************/
void DEPG0150BxS810FxX_BW::EPD_DeepSleep(void) {
  SendCommand(0x10);
  SendData(0x01);   
  DelayMs(100); 
}

/******************************** EPD_ALL_image ******************************************/
void DEPG0150BxS810FxX_BW::EPD_ALL_image(const unsigned char *datas) {

  SendCommand(0x24);   //write RAM for black(0)/white (1)

  for (int i = 0; i < ALLSCREEN_BYTES; i++) {
    SendData(pgm_read_byte(&datas[i]));
  }	
  EPD_Update();
}

/********************************** All black *********************************************/
void DEPG0150BxS810FxX_BW::EPD_WhiteScreen_Black(void) {
  EPD_Load_Data(0x00);
}

/********************************** All white *********************************************/
void DEPG0150BxS810FxX_BW::EPD_WhiteScreen_White(void) {
  EPD_Load_Data(0xff);
}

/********************************** Load Data *********************************************/
void DEPG0150BxS810FxX_BW::EPD_Load_Data(unsigned char data) {
  SendCommand(0x24);   //write RAM for black(0)/white (1)

  for (int i = 0; i < ALLSCREEN_BYTES; i++) {
    SendData(data);
  }	

  EPD_Update();
}


void DEPG0150BxS810FxX_BW::HalLcd_Partial(void)
{
	Reset();
	WaitUntilIdle();
	Epaper_LUT_By_MCU(0x25,(unsigned char *)WF_PARTIAL);//外加局刷波形
  SendCommand(0x37);  //  局刷功能开启， pingpong 模式使能
  SendData(0x00);  
  SendData(0x00);  
  SendData(0x00);  
  SendData(0x00); 
  SendData(0x00);  	
  SendData(0x40);  
  SendData(0x00);  
  SendData(0x00);   
  SendData(0x00);  
  SendData(0x00);
 

  SendCommand(0x3C);  //  border设定
  SendData(0x80);   
	
//  SendCommand(0x22); 
//  SendData(0xC0);   
//  SendCommand(0x20); 
//  WaitUntilIdle();  

}

void DEPG0150BxS810FxX_BW::Epaper_LUT_By_MCU(unsigned char vcom,unsigned char *wave_data)
{
	Epaper_LUT((unsigned char*)wave_data);      // 送入波形   give the waveform

	SendCommand(0x3F); 
	SendData(*(wave_data+153));

	SendCommand(0x03);      //门电压   gate voltage  
	SendData(*(wave_data+154));

	SendCommand(0x04);      //源电压   source voltage   
	SendData(*(wave_data+155)); 
	SendData(*(wave_data+156));
	SendData(*(wave_data+157));

	SendCommand(0x2C);         ///vcom   
	SendData(*(wave_data+158));
}

void DEPG0150BxS810FxX_BW::Epaper_LUT(unsigned char * wave_data)
{        
  unsigned char count;
  SendCommand(0x32);
  for(count=0;count<153;count++) SendData(*wave_data++); 
  WaitUntilIdle();
}

void DEPG0150BxS810FxX_BW::HalLcd_Partial_Update(void)
{
  SendCommand(0x22);   //  局部刷新
  SendData(0xCF);    // 外加局刷， 非OTP调用
  SendCommand(0x20); 
  WaitUntilIdle(); 	
}

/** DEPG0150BxS810FxX_BW.cpp END **/
/**
 *  @brief: private function to specify the memory area for data R/W
 */
void DEPG0150BxS810FxX_BW::SetMemoryArea(int x_start, int y_start, int x_end, int y_end)
{
	SendCommand(0x11); //data entry mode
  SendData(0x03);
	SendCommand(0x44);
	/* x point must be the multiple of 8 or the last 3 bits will be ignored */
	SendData((x_start >> 3) & 0xFF);
  SendData((x_end >> 3) & 0xFF);
	SendCommand(0x45);
	SendData(y_start & 0xFF);
	SendData((y_start >> 8) & 0xFF);
	SendData(y_end & 0xFF);
	SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void DEPG0150BxS810FxX_BW::SetMemoryPointer(int x, int y)
{
	SendCommand(0x4e);
	/* x point must be the multiple of 8 or the last 3 bits will be ignored */
	SendData((x >> 3) & 0xFF);
	SendCommand(0x4F);
	SendData(y & 0xFF);
	SendData((y >> 8) & 0xFF);
	WaitUntilIdle();
}

void DEPG0150BxS810FxX_BW::SetFrameMemory(
        const unsigned char* image_buffer,
        int x,
        int y,
        int image_width,
        int image_height
)
{
	int x_end;
	int y_end;
	
	// DigitalWrite(reset_pin, LOW);                //module reset
	// DelayMs(10);
	// DigitalWrite(reset_pin, HIGH);
	// DelayMs(10);
	// SendCommand(0x3c);
	// SendData(0x80);

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
	if (x + image_width >= EPD_WIDTH) {
		x_end = EPD_WIDTH - 1;
	} else {
		x_end = x + image_width - 1;
	}
	if (y + image_height >= EPD_HEIGHT) {
		y_end = EPD_HEIGHT - 1;
	} else {
		y_end = y + image_height - 1;
	}

	SetMemoryArea(x, y, x_end, y_end);
	SetMemoryPointer(x, y);
	SendCommand(0x24);

	/* send the image data */
	for (int j = 0; j < y_end - y + 1; j++) {
		for (int i = 0; i < (x_end - x + 1) / 8; i++) {
			SendData(image_buffer[i + j * (image_width / 8)]);
		}
	}
}

void DEPG0150BxS810FxX_BW::DisplayPartBaseImage(const unsigned char* frame_buffer) {
	int w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
	int h = EPD_HEIGHT;

	if (frame_buffer != NULL) {
		SendCommand(0x24);
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				SendData(pgm_read_byte(&frame_buffer[i + j * w]));
			}
		}

		SendCommand(0x26);
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				SendData(pgm_read_byte(&frame_buffer[i + j * w]));
			}
		}
	}

	EPD_Update();
}

void DEPG0150BxS810FxX_BW::Clear(void)
{
	int w, h;
	w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
	h = EPD_HEIGHT;
 
	SendCommand(0x24);
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			SendData(0xff);
		}
	}
	//DISPLAY REFRESH
	EPD_Update();
}
