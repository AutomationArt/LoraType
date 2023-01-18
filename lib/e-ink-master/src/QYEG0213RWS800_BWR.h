#ifndef __EPD_213_H__
#define __EPD_213_H__
#include "EpdBase.h"

//250*122///////////////////////////////////////


#define ALLSCREEN_GRAGHBYTES	4000

#define MONO 1                  
#define RED  2                  

/* Display resolution */
#define EPD_WIDTH       122
#define EPD_HEIGHT      250

class QYEG0213RWS800_BWR:EpdBase {
    public:
        QYEG0213RWS800_BWR(uint8_t _rst, uint8_t _dc, uint8_t _cs,  uint8_t _busy, int8_t _clk=-1, uint8_t _spi_num = 0 ,uint32_t _freq = 2000000): \
        EpdBase(_rst, _dc, _cs, _busy, _clk, _spi_num, _freq){}
        
        ~QYEG0213RWS800_BWR(){}

    public:
        void EpaperIO_Init(void);
        void Epaper_READBUSY(void);


        void EPD_HW_Init(void); //Electronic paper initialization
        void EPD_Update(void);
            
        void EPD_WhiteScreen_Red(void);
        void EPD_WhiteScreen_Black(void);
        void EPD_WhiteScreen_White(void);
        void EPD_DeepSleep(void);
        //Display 

        void EPD_ALL_image(const unsigned char *datas1,const unsigned char *datas2);
        void EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,const unsigned char color_mode,unsigned int PART_COLUMN,unsigned int PART_LINE);
        void EPD_Dis_Part_mult(unsigned int x_startA,unsigned int y_startA,const unsigned char * datasA1,const unsigned char * datasA2,
                                unsigned int x_startB,unsigned int y_startB,const unsigned char * datasB1,const unsigned char * datasB2,
                                                    unsigned int PART_COLUMN,unsigned int PART_LINE);
};

#endif