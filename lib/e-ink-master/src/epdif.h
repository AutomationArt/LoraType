#ifndef EPDIF_H
#define EPDIF_H

#include <Arduino.h>
#include <SPI.h>
#include "imagedata.h"

#if defined( ESP32 )
// Pin definition
#define RST_PIN         16
#define DC_PIN          22
#define CS_PIN          18
#define BUSY_PIN        23
#define CLK_PIN         5
#define SDI_PIN         27

#elif defined( AVR )
#define RST_PIN         8
#define DC_PIN          9
#define CS_PIN          10
#define BUSY_PIN        7

#elif defined( ESP8266 )
#define RST_PIN         16
#define DC_PIN          4
#define CS_PIN          15
#define BUSY_PIN        5
#define CLK_PIN         14
#define SDI_PIN         13

#elif defined( CubeCell_Board )//AB01
#define RST_PIN         GPIO1
#define DC_PIN          GPIO2
#define CS_PIN          GPIO3
#define BUSY_PIN        GPIO5
#endif

class EpdIf {
public:
    EpdIf(void);
    ~EpdIf(void);

    static int  IfInit(void);
    static void DigitalWrite(int pin, int value); 
    static int  DigitalRead(int pin);
    static void DelayMs(unsigned int delaytime);
    static void SpiTransfer(unsigned char data);
};

#endif
