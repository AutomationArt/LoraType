#ifndef __DEPG0213Bx800FxX_BW_H__
#define __DEPG0213Bx800FxX_BW_H__
#include "EpdBase.h"

//250*122///////////////////////////////////////


#define ALLSCREEN_GRAGHBYTES	4000

/* Display resolution */
#define EPD_WIDTH_213       122
#define EPD_HEIGHT_213      250   

class DEPG0213Bx800FxX_BW:public EpdBase {
    public:
        DEPG0213Bx800FxX_BW(uint8_t _rst, uint8_t _dc, uint8_t _cs,  uint8_t _busy, int8_t _clk=-1, uint8_t _spi_num = 0 ,uint32_t _freq = 2000000): \
        EpdBase(_rst, _dc, _cs, _busy, _clk, _spi_num, _freq){}
        ~DEPG0213Bx800FxX_BW(){}

    public:
        void EPD_Init(void); //Electronic paper initialization
        void EPD_Update(void);
        void EPD_WhiteScreen_Black(void);
        void EPD_WhiteScreen_White(void);
        void EPD_DeepSleep(void);
        //Display         
        void Clear(void);
        void EPD_ALL_image(const unsigned char *datas);
        void HalLcd_Partial_Update(void);
        void SetFrameMemory(const unsigned char* image_buffer,int x,int y,int image_width,int image_height);
        void ClearFrameMemory(unsigned char color);
    private:
        void EPD_Load_Data(unsigned char data);
        void SetMemoryArea(int x_start, int y_start, int x_end, int y_end);
        void SetMemoryPointer(int x, int y);
};

#endif