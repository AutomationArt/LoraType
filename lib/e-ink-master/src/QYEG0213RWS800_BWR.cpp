#include "QYEG0213RWS800_BWR.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

void QYEG0213RWS800_BWR::EPD_HW_Init(void)
{
	/* this calls the peripheral hardware interface, see epdif */
	//Serial.begin(115200);
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
	// Reset();
	WaitUntilIdle();
	SendCommand(0x12); // soft reset
	WaitUntilIdle();

	SendCommand(0x74); //set analog block control       
	SendData(0x54);
	SendCommand(0x7E); //set digital block control          
	SendData(0x3B);

	SendCommand(0x01); //Driver output control      
	SendData(0xF9);
	SendData(0x00);
	SendData(0x00);

	SendCommand(0x11); //data entry mode       
	SendData(0x01);

	SendCommand(0x44); //set Ram-X address start/end position   
	SendData(0x01);
	SendData(0x10);    //0x0F-->(15+1)*8=128

	SendCommand(0x45); //set Ram-Y address start/end position          
	SendData(0xF9);   //0xF9-->(249+1)=250
	SendData(0x00);
	SendData(0x00);
	SendData(0x00); 

	SendCommand(0x3C); //BorderWavefrom
	SendData(0x01);	

  SendCommand(0x18); 
	SendData(0x80);	

	SendCommand(0x4E);   // set RAM x address count to 0;
	SendData(0x01);
	SendCommand(0x4F);   // set RAM y address count to 0xF9-->(249+1)=250;    
	SendData(0xF9);
	SendData(0x00);
	// Epaper_READBUSY();
	WaitUntilIdle();
	Serial.println("e-Paper init OK!");	
}

//////////////////////////////All screen update////////////////////////////////////////////
void QYEG0213RWS800_BWR::EPD_ALL_image(const unsigned char *datas1,const unsigned char *datas2)
{
   	unsigned int i;
    SendCommand(0x24);   //write RAM for black(0)/white (1)

	for (int i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        SendData(pgm_read_byte(&datas1[i]));
    }	
    SendCommand(0x26);   //write RAM for red(1)/white (0)

	for (int i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        SendData(pgm_read_byte(&datas2[i]));
    }
   	EPD_Update();		 
}

/////////////////////////////////////////////////////////////////////////////////////////
void QYEG0213RWS800_BWR::EPD_Update(void)
{   
	SendCommand(0x22); 
	SendData(0xF7);   
	SendCommand(0x20); 
	// Epaper_READBUSY();
	// WaitUntilIdle();   
}

void QYEG0213RWS800_BWR::EPD_DeepSleep(void)
{  	
  SendCommand(0x10); //enter deep sleep
  SendData(0x01); 
}

/////////////////////////////////Single display////////////////////////////////////////////////
void QYEG0213RWS800_BWR::EPD_WhiteScreen_Red(void)
{
   unsigned int i,k;
    SendCommand(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0xff);
			}
  }
	
    SendCommand(0x26);   //write RAM for red(1)/white (0)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0xff);
			}
  }
	
	EPD_Update();
}



void QYEG0213RWS800_BWR::EPD_WhiteScreen_Black(void)

{
   unsigned int i,k;
    SendCommand(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0x00);
			}
  }
	
    SendCommand(0x26);   //write RAM for red(1)/white (0)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0x00);
			}
  }
	
		EPD_Update();
}




void QYEG0213RWS800_BWR::EPD_WhiteScreen_White(void)

{
   unsigned int i,k;
    SendCommand(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0xff);
			}
  }
	
    SendCommand(0x26);   //write RAM for red(1)/white (0)
	for(k=0;k<250;k++)
  {
		for(i=0;i<16;i++)
		{
			SendData(0x00);
			}
  }
	
	EPD_Update();
}


///////////////////////////Part update//////////////////////////////////////////////
void QYEG0213RWS800_BWR::EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,const unsigned char color_mode,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
	unsigned int i; 
	unsigned int x_end,y_start1,y_start2,y_end1,y_end2;
	
	x_start=(x_start+8)/8;
	x_end=x_start+PART_LINE/8-1; 
	
	y_start1=0;
	y_start2=y_start;
	if(y_start>=256)
	{
		y_start1=y_start2/256;
		y_start2=y_start2%256;
	}
	y_end1=0;
	y_end2=y_start+PART_COLUMN-1;
	if(y_end2>=256)
	{
		y_end1=y_end2/256;
		y_end2=y_end2%256;		
	}		
	
	SendCommand(0x44);       // set RAM x address start/end, in page 35
	SendData(x_start);    		// RAM x address start
	SendData(x_end);    			// RAM x address end
	SendCommand(0x45);       // set RAM y address start/end, in page 35
	SendData(y_start2);    	// RAM y address start
	SendData(y_start1);   	 	// RAM y address start
	SendData(y_end2);    		// RAM y address end
	SendData(y_end1);    		// RAM y address end


	SendCommand(0x4E);   // set RAM x address count to 0;
	SendData(x_start); 
	SendCommand(0x4F);   // set RAM y address count to 0X127;    
	SendData(y_start2);
	SendData(y_start1);

  if(color_mode==MONO)
    SendCommand(0x24);   //write RAM for black(0)/white (1)

  if(color_mode==RED)
    SendCommand(0x26);   //write RAM for black(0)/white (1)

	for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     SendData(* datas);
			datas++;
   } 

	EPD_Update();

}



void QYEG0213RWS800_BWR::EPD_Dis_Part_mult(unsigned int x_startA,unsigned int y_startA,const unsigned char * datasA1,const unsigned char * datasA2,
	                     unsigned int x_startB,unsigned int y_startB,const unsigned char * datasB1,const unsigned char * datasB2,
											 unsigned int PART_COLUMN,unsigned int PART_LINE)
{
	unsigned int i;  
	unsigned int x_endA,y_startA1,y_startA2,y_endA1,y_endA2;
	unsigned int x_endB,y_startB1,y_startB2,y_endB1,y_endB2;	
	
	//Data A////////////////////////////
	x_startA=(x_startA+8)/8;
	x_endA=x_startA+PART_LINE/8-1; 
	
	y_startA1=0;
	y_startA2=y_startA;
	if(y_startA2>=256)
	{
		y_startA1=y_startA2/256;
		y_startA2=y_startA2%256;
	}
	y_endA1=0;
	y_endA2=y_startA+PART_COLUMN-1;
	if(y_endA2>=256)
	{
		y_endA1=y_endA2/256;
		y_endA2=y_endA2%256;		
	}		
	
	SendCommand(0x44);       // set RAM x address start/end, in page 35
	SendData(x_startA);    // RAM x address start at 00h;
	SendData(x_endA);    // RAM x address end at 0fh(15+1)*8->128 
	SendCommand(0x45);       // set RAM y address start/end, in page 35
	SendData(y_startA2);    // RAM y address start at 0127h;
	SendData(y_startA1);    // RAM y address start at 0127h;
	SendData(y_endA2);    // RAM y address end at 00h;
	SendData(y_endA1);   

	SendCommand(0x4E);   // set RAM x address count to 0;
	SendData(x_startA); 
	SendCommand(0x4F);   // set RAM y address count to 0X127;    
	SendData(y_startA2);
	SendData(y_startA1);
	
	 SendCommand(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     SendData(*datasA1);
			datasA1++;
   } 
	 
	 SendCommand(0x26);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     SendData(*datasA2);
			datasA2++;
   } 
	 
	//Data B/////////////////////////////////////
	x_startB=(x_startB+8)/8;
	x_endB=x_startB+PART_LINE/8-1; 
	
	y_startB1=0;
	y_startB2=y_startB;
	if(y_startB2>=256)
	{
		y_startB1=y_startB2/256;
		y_startB2=y_startB2%256;
	}
	y_endB1=0;
	y_endB2=y_startB+PART_COLUMN-1;
	if(y_endB2>=256)
	{
		y_endB1=y_endB2/256;
		y_endB2=y_endB2%256;		
	}		
	
	SendCommand(0x44);       // set RAM x address start/end, in page 35
	SendData(x_startB);    // RAM x address start at 00h;
	SendData(x_endB);    // RAM x address end at 0fh(15+1)*8->128 
	SendCommand(0x45);       // set RAM y address start/end, in page 35
	SendData(y_startB2);    // RAM y address start at 0127h;
	SendData(y_startB1);    // RAM y address start at 0127h;
	SendData(y_endB2);    // RAM y address end at 00h;
	SendData(y_endB1);    

	SendCommand(0x4E);   // set RAM x address count to 0;
	SendData(x_startB); 
	SendCommand(0x4F);   // set RAM y address count to 0X127;    
	SendData(y_startB2);
	SendData(y_startB1);
	
	 SendCommand(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     SendData(*datasB1);
			datasB1++;
   } 	

	 SendCommand(0x26);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     SendData(*datasB2);
			datasB2++;
   } 		 
	 
	 EPD_Update();

}

/*QYEG0213RWS800_BWR.cpp END*/