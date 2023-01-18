/**
  *This example is the latest 2.13 black and white red ink screen code.
  *Compared with the previous 2.13 black and white red Tri Color ink screen, the biggest change is that the pixel point is changed from 212x104 to 250x122.
  *When selecting this program, please pay attention to whether the ink screen in your hand is the latest 2.13 black and white red ink screen.
  */

#include "QYEG0213RWS800_BWR.h"
#include "picture.h"

#define UNDEFINED -1
#ifdef CubeCell_Board //AB01
    #define FREQUENCY 2000000

    QYEG0213RWS800_BWR epd213bwr(GPIO1, GPIO2, GPIO3, GPIO5, UNDEFINED, SPI_NUM_0, FREQUENCY);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin, spi_num, frequency
#elif defined( ESP32 )
    #define RST_PIN         16
    #define DC_PIN          22
    #define CS_PIN          18
    #define BUSY_PIN        23
    #define CLK_PIN         5

    QYEG0213RWS800_BWR epd213bwr(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined( ESP8266 )
    #define RST_PIN         16
    #define DC_PIN          4
    #define CS_PIN          15
    #define BUSY_PIN        5
    #define CLK_PIN         14

    QYEG0213RWS800_BWR epd213bwr(RST_PIN, DC_PIN, CS_PIN, BUSY_PIN, CLK_PIN);//reset_pin, dc_pin, cs_pin, busy_pin, clk_pin
#elif defined ( AVR )
    #define RST_PIN         8
    #define DC_PIN          9
    #define CS_PIN          10
    #define BUSY_PIN        7
#endif

void setup() {
    Serial.begin(115200);
    epd213bwr.EPD_HW_Init(); //Electronic paper initialization
    epd213bwr.EPD_ALL_image(gImage_213black,gImage_213red);	//Refresh the picture in full screen
    epd213bwr.EPD_DeepSleep();  //Enter deep sleep	

    // delay(5000);
    // epd213bwr.EPD_HW_Init();
    // epd213bwr.EPD_WhiteScreen_Red();
    // delay(5000);
    // epd213bwr.EPD_HW_Init();
    // epd213bwr.EPD_WhiteScreen_Black();
    // delay(5000);
    // epd213bwr.EPD_HW_Init();
    // epd213bwr.EPD_WhiteScreen_White();
    epd213bwr.EPD_DeepSleep();
}
void loop() {

}