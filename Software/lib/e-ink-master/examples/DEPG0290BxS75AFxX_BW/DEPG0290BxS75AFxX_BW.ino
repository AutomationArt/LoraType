/*
DEPG0290BxS75AFxX_BW is a 2.9 inch black and white screen
Display Resolution 128(H)×296(V)
*/
#include "DEPG0290BxS75AFxX_BW.h"
#include "picture.h"
#include "e_ink_display.h"


#define UNDEFINED -1
#ifdef CubeCell_Board //AB01
    #define FREQUENCY 2000000

    DEPG0290BxS75AFxX_BW epd290bw(GPIO1, GPIO2, GPIO3, GPIO5, UNDEFINED, SPI_NUM_0, FREQUENCY);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin, spi_num, frequency
#elif defined( ESP32 )
    #define RST_PIN         25
    #define DC_PIN          13
    #define CS_PIN          15
    #define BUSY_PIN        17
    #define CLK_PIN         16

    DEPG0290BxS75AFxX_BW epd290bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined( ESP8266 )
    #define RST_PIN         16
    #define DC_PIN          4
    #define CS_PIN          15
    #define BUSY_PIN        5
    #define CLK_PIN         14

    DEPG0290BxS75AFxX_BW epd290bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined ( AVR )
    #define RST_PIN         8
    #define DC_PIN          9
    #define CS_PIN          10
    #define BUSY_PIN        7
#endif

unsigned char img[1024*5];
Paint pt(img, 0, 0);
unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1



void setup() {
    Serial.begin(115200);
    epd290bw.EPD_Init(); //Electronic paper initialization
    epd290bw.EPD_ALL_image(gImage_290image_bw);	//Refresh the picture in full screen
    
    delay(2000);
    pt.Clear(COLORED);
    pt.SetWidth(200);
    pt.SetHeight(24);
    pt.DrawStringAt(0, 0, "E-ink - 2.90!", &Font16, UNCOLORED);
    epd290bw.SetFrameMemory(pt.GetImage(), 0, 10, pt.GetWidth(), pt.GetHeight());
    epd290bw.HalLcd_Partial_Update();
    delay(2000);

    pt.SetWidth(64);
	pt.SetHeight(64);
	pt.Clear(UNCOLORED);
  pt.DrawQrcode(0,0,"https://heltec.org/",4);
	epd290bw.SetFrameMemory(pt.GetImage(), 10, 10, pt.GetWidth(), pt.GetHeight());
    epd290bw.HalLcd_Partial_Update();
    delay(100);
    epd290bw.EPD_DeepSleep();  //Enter deep sleep	

}
void loop() {

}