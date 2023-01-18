#ifndef __SELECT_H__
#define __SELECT_H__

#include <SPI.h>
#include "e_ink_display.h"
#include "e_ink.h"

#define COLORED     0
#define UNCOLORED   1

void Display(void);
void Display_clear(void);
void Display_String(int width, int Heigh, int TextX, int TextY, const char* text, sFONT* font, int areaX,int aeraY );
#if defined ( USE_213_BWR ) || defined ( USE_213_BWY ) || defined ( USE_154_BWY ) || \
    defined ( USE_154_BWR_152 )  || defined ( USE_260_BWR ) || \
    defined ( USE_270_BWR ) || defined ( USE_290_BWR ) || defined ( USE_290_BWY ) || \
    defined ( USE_420_BWR ) || defined ( USE_420_BWY )
void Display_picture(const unsigned char* IMAGE_BLACK,const unsigned char* IMAGE_RED);
#elif   defined ( USE_213_BWSoft ) || defined ( USE_583_BW ) || defined ( USE_583_THREE_COLORS ) || \
        defined ( USE_750_BWR ) || defined ( USE_750_BWY ) || \
        defined ( USE_750_BW  ) || defined ( USE_270_BW ) || defined ( USE_290_BWSoft ) || \
        defined ( USE_420_BW ) || defined ( USE_213_BW ) || defined ( USE_260_BW )
void Display_picture(const unsigned char* IMAGE_BLACK);
#elif defined ( USE_154_BW_GREEN ) || defined ( USE_290_BW ) || defined ( USE_154_BW_BLUE )
void Display_picture(const unsigned char* IMAGE_BLACK);
#endif

#endif /* SELECT FILE END */ 