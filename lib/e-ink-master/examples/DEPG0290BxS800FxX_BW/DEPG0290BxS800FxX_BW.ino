/*
DEPG0290BxS75AFxX_BW is a 2.9 inch black and white screen
Display Resolution 128(H)×296(V)
*/
#include "DEPG0290BxS800FxX_BW.h"
#include "picture.h"

#define UNDEFINED -1
#ifdef CubeCell_Board //AB01
    #define FREQUENCY 2000000

    DEPG0290BxS800FxX_BW epd290bw(GPIO1, GPIO2, GPIO3, GPIO5, UNDEFINED, SPI_NUM_0, FREQUENCY);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin, spi_num, frequency
#elif defined( ESP32 )
    #define RST_PIN         16
    #define DC_PIN          22
    #define CS_PIN          18
    #define BUSY_PIN        23
    #define CLK_PIN         5

    DEPG0290BxS800FxX_BW epd290bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined( ESP8266 )
    #define RST_PIN         16
    #define DC_PIN          4
    #define CS_PIN          15
    #define BUSY_PIN        5
    #define CLK_PIN         14

    DEPG0290BxS800FxX_BW epd290bw(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined ( AVR )
    #define RST_PIN         8
    #define DC_PIN          9
    #define CS_PIN          10
    #define BUSY_PIN        7
#endif

void setup() {
    Serial.begin(115200);
    epd290bw.EPD_Init(); //Electronic paper initialization
    epd290bw.EPD_ALL_image(gImage_290image_bw);	//Refresh the picture in full screen
    
    // delay(1000);
    // epd290bw.EPD_WhiteScreen_Black();
    // delay(1000);
    // epd290bw.EPD_WhiteScreen_White();

    /**
    Remark:
    To Exit Deep Sleep mode, User required
    to send HWRESET to the driver
    */
    // epd290bw.EPD_DeepSleep();  //Enter deep sleep

}
void loop() {

}