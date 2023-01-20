/*
DEPG0213Bx800FxX_BW is a 2.13 inch black and white screen
Display Resolution 122(H)×250(V) 
*/
#include "DEPG0213Bx800FxX_BW.h"
#include "picture.h"
#include "e_ink_display.h"

#define UNDEFINED -1
#ifdef CubeCell_Board //AB01
    #define FREQUENCY 2000000

    DEPG0213Bx800FxX_BW epd213bw(GPIO1, GPIO2, GPIO3, GPIO5, UNDEFINED, SPI_NUM_0, FREQUENCY);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin, spi_num, frequency
#elif defined( ESP32 )
    #define RST_PIN         4
    #define DC_PIN          13
    #define CS_PIN          15
    #define BUSY_PIN        17
    #define CLK_PIN         16

    DEPG0213Bx800FxX_BW epd213bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined( ESP8266 )
    #define RST_PIN         16
    #define DC_PIN          4
    #define CS_PIN          15
    #define BUSY_PIN        5
    #define CLK_PIN         14
    
    DEPG0213Bx800FxX_BW epd213bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined ( AVR )
    #define RST_PIN         8
    #define DC_PIN          9
    #define CS_PIN          10
    #define BUSY_PIN        7
#endif


unsigned char img[1024];
Paint pt(img, 0, 0);
unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1




void setup() {
    Serial.begin(115200);
    epd213bw.EPD_Init(); //Electronic paper initialization
    epd213bw.EPD_ALL_image(gImage_213image_bw);	//Refresh the picture in full screen
    delay(2000);
    pt.Clear(COLORED);
    pt.SetWidth(100);
    pt.SetHeight(124);
    pt.DrawStringAt(10,11, "2.13!", &Font16, UNCOLORED);
    epd213bw.SetFrameMemory(pt.GetImage(), 0, 0, pt.GetWidth(), pt.GetHeight());
    epd213bw.HalLcd_Partial_Update();
    delay(2000);

    pt.SetWidth(64);
	pt.SetHeight(64);

	pt.Clear(UNCOLORED);
	pt.DrawRectangle(0, 0, 40, 50, COLORED);
	pt.DrawLine(0, 0, 40, 50, COLORED);
	pt.DrawLine(40, 0, 0, 50, COLORED);
	epd213bw.SetFrameMemory(pt.GetImage(), 16, 60, pt.GetWidth(), pt.GetHeight());
    epd213bw.HalLcd_Partial_Update();
    delay(100);
    epd213bw.EPD_DeepSleep();  //Enter deep sleep
}
void loop() {

}