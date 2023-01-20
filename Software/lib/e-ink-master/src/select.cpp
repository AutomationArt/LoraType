#include "select.h"
#include "imagedata.h"

unsigned char image[1024];
Paint paint(image, 0, 0);    // width should be the multiple of 8
const unsigned char* _imageBuf;//not have actually value
/* The preprocessing function of screen is defined. */
void Display(void) {
#if defined ( USE_154_BW_BLUE )
    Display_picture(IMAGE_DATA2);
    Display_clear();
    Display_picture(IMAGE_DATA);
    Display_clear();
    Display_picture(IMAGE_DATA1);
    Display_clear();
    Display_String(30,170,20,8,"E-INK-1.54",&Font20,90,15);
#elif defined ( USE_154_BW_GREEN )
    Display_picture(IMAGE_154_BW);
    // Display_clear();
    // Display_String(30,170,20,8,"E-INK-1.54",&Font20,90,15);
#elif defined ( USE_154_BWY )
    Display_picture(IMAGE_154_BWY_2_BLACK,IMAGE_154_BWY_2_RED);
#elif defined ( USE_154_BWR_152 )
    Display_picture(IMAGE_154_BWR_2_BLACK,IMAGE_154_BWR_2_RED);
#elif defined ( USE_213_BW )
    Display_picture(IMAGE_213_BW_1);
    // Display_picture(gImage_IMAGE,_imageBuf);
#elif defined ( USE_213_BWR )
    Display_picture(IMAGE_BLACK,IMAGE_RED);
#elif defined ( USE_213_BWY )
    Display_picture(IMAGE_213_BWY_BLACK,IMAGE_213_BWY_RED);
#elif defined(USE_213_BWSoft)
    Display_picture(IMAGE_213_BW_1);
#elif defined ( USE_260_BW )
    Display_picture( IMAGE_260_BW_BLACK );
#elif defined ( USE_260_BWR )
    Display_picture(IMAGE_260_BWR_BLACK,IMAGE_260_BWR_RED);
#elif defined ( USE_270_BW )
    Display_picture(IMAGE_270_BW_BLACK);
#elif defined ( USE_270_BWR )
    Display_picture(IMAGE_270_BWR_2_BLACK,IMAGE_270_BWR_2_RED);
#elif defined ( USE_290_BW )
    Display_picture(IMAGE2);
    Display_clear();
    Display_picture(IMAGE1);
    Display_clear();
    Display_picture(IMAGE);
    Display_String(24,128,10,8,"E-INK-2.90",&Font16,50,90);
#elif defined ( USE_290_BWR )
    Display_picture(IMAGE_290_BWR_BLACK,IMAGE_290_BWR_RED);
#elif defined ( USE_290_BWY )
    Display_picture(IMAGE_290_BWY_BLACK,IMAGE_290_BWY_RED);
#elif defined ( USE_290_BWSoft )
    Display_picture(IMAGE_290_BWSoft);
#elif defined ( USE_420_BWR )
    Display_picture(IMAGE_420_BWY_1_BLACK,IMAGE_420_BWY_1_RED);
#elif defined ( USE_420_BWY )
    Display_picture(IMAGE_420_BWY_1_BLACK,IMAGE_420_BWY_1_RED);
#elif defined ( USE_420_BW )
    Display_picture(IMAGE_420_BW_1);
#endif
#ifndef AVR
    #if defined ( USE_583_THREE_COLORS )//5.83 inch three colors
        Display_picture( IMAGE_583_THREE_COLORS );
    #elif defined ( USE_583_BW )
         Display_picture( IMAGE_583_BW_1 );
    //#elif defined ( USE_583_BWY )
    //    Display_picture( IMAGE_583_BWY );
    #elif defined ( USE_750_BWR )
        Display_picture( IMAGE_750_BWR );
    #elif defined ( USE_750_BWY )
        Display_picture( IMAGE_750_BWY );
    #elif defined ( USE_750_BW )
        Display_picture( IMAGE_750_BW );
    #endif
#endif
}
void Display_clear() {
#if defined ( USE_260_BW  ) || defined ( USE_260_BWR ) || defined ( USE_270_BW  ) || \
    defined ( USE_270_BWR ) || defined ( USE_290_BWSoft ) || defined ( USE_290_BWY ) || \
    defined ( USE_290_BWR ) || defined ( USE_420_BW  ) || defined ( USE_420_BWR ) || \
    defined ( USE_420_BWY ) || defined ( USE_154_BWY ) || defined ( USE_154_BWR_152 ) || \
    defined (  USE_213_BWR  ) || defined ( USE_213_BWY ) || defined ( USE_213_BWSoft ) || \
    defined ( USE_213_BW ) || defined ( USE_583_BW ) || defined ( USE_583_THREE_COLORS ) || \
    defined ( USE_750_BW ) || defined ( USE_750_BWR ) ||  defined ( USE_750_BWY )
	epd.ClearFrame();
#elif defined ( USE_154_BW_GREEN ) || defined ( USE_290_BW ) || \
      defined ( USE_154_BW_BLUE )
    epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
    #if defined (USE_290_BW)
        epd.DisplayFrame();
        epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
        epd.DisplayFrame();
    #endif
    delay(2000); 
#endif
}

#if defined ( USE_213_BWR ) || defined ( USE_213_BWY ) || defined ( USE_154_BWY ) || \
    defined ( USE_154_BWR_152 )  || defined ( USE_260_BWR ) || \
    defined ( USE_270_BWR ) || defined ( USE_290_BWR ) || defined ( USE_290_BWY ) || \
    defined ( USE_420_BWR ) || defined ( USE_420_BWY )
void Display_picture(const unsigned char* IMAGE_BLACK,const unsigned char* IMAGE_RED) {
    epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);
}
#elif   defined ( USE_213_BWSoft ) || defined ( USE_583_BW ) || defined ( USE_583_THREE_COLORS ) || \
        defined ( USE_750_BWR ) || defined ( USE_750_BWY ) || defined ( USE_750_BW  ) || \
         defined ( USE_270_BW ) || defined ( USE_290_BWSoft ) ||  defined ( USE_420_BW ) || \
         defined ( USE_213_BW ) || defined ( USE_260_BW )
void Display_picture(const unsigned char* IMAGE_BLACK) {
    epd.DisplayFrame(IMAGE_BLACK);
}
#elif defined ( USE_154_BW_GREEN ) || defined ( USE_290_BW ) || defined ( USE_154_BW_BLUE )
void Display_picture(const unsigned char* IMAGE_BLACK) {
    #if defined (USE_290_BW)
        epd.SetFrameMemory(IMAGE_BLACK);
        epd.DisplayFrame();
    #endif
    epd.SetFrameMemory(IMAGE_BLACK);
    epd.DisplayFrame();
    delay(2000);
}
#endif


/**
	Width,Heigh displays the fast read and height of the area.
	The starting position of the character in the display area of the TextX,TextY.
	Text is the display content.
	Font is the display font: font8, font12, font16, font20, font24.
	The starting position of the areaX,areaY display area relative to the screen is (0,0) in the upper right corner.
 * */

void Display_String(int width, int Heigh, int TextX, int TextY, const char* text, sFONT* font, int areaX,int aeraY ) {
    paint.Clear(COLORED);
    paint.SetWidth(width); /* Set the size of the area */
    paint.SetHeight(Heigh);
    paint.SetRotate(ROTATE_270); /* rotate the direction of the text */
    paint.DrawStringAt(TextX, TextY, text, font, UNCOLORED);
#if defined(USE_213_BWR) 
    //epd.SetPartialWindowBlack(paint.GetImage(), areaX, aeraY, paint.GetWidth(), paint.GetHeight());
    epd.SetPartialWindowRed(paint.GetImage(), areaX, aeraY, paint.GetWidth(), paint.GetHeight());
#elif defined(USE_154_BW_BLUE) || defined(USE_290_BW) || defined(USE_154_BW_GREEN)
    epd.SetFrameMemory(paint.GetImage(), areaX,aeraY, paint.GetWidth(), paint.GetHeight());
#endif
    epd.DisplayFrame();
    delay(1000);
}

/* SELECT FILE END */