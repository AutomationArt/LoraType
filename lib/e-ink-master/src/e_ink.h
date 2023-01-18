#ifndef E_INK_H
#define E_INK_H

#include <Arduino.h>
#include "epdif.h"
#include <Wire.h>
#include <SPI.h>
#include "imagedata.h"

/**
 *
 *Please select a screen macro definition
 */
 #define USE_290_BW  
        /* USE_154_BW_GREEN, USE_154_BW_BLUE, USE_154_BWY, USE_154_BWR_152, 
           USE_213_BW, USE_213_BWSoft, USE_213_BWR, USE_213_BWY, 
           USE_260_BW, USE_260_BWR, 
           USE_270_BW, USE_270_BWR, 
           USE_290_BW, USE_290_BWSoft, USE_290_BWR, USE_290_BWY,
           USE_420_BW, USE_420_BWR, USE_420_BWY,
           USE_583_BW, USE_583_BWY, USE_583_BWR,
           USE_750_BW, USE_750_BWR, USE_750_BWY
           */
/* 1.54 inch screen of 152x152 BWY and BWR */
#define USE_154 defined ( USE_154_BWY ) || defined ( USE_154_BWR_152 )//1.54inch,152x152 pixel with BLACK,WHITE and RED does not exist.
/* 2.13 inch screen  BW and BWSoft and BWY and BWR */
#define USE_213 defined (  USE_213_BWR  ) || defined ( USE_213_BWY ) || \
                defined ( USE_213_BWSoft ) || defined ( USE_213_BW )
/* 2.6 inch screen  BW and BWR */
#define USE_260 defined (  USE_260_BW  ) || defined (  USE_260_BWR  )
/* 2.7 inch screen  BW and BWR */
#define USE_270 defined ( USE_270_BW ) || defined ( USE_270_BWR )
/* 2.9 inch screen BWSoft and BWY and BWR */
#define USE_290 defined ( USE_290_BWSoft ) || defined ( USE_290_BWR ) || \
                defined ( USE_290_BWY )
/* 4.2 inch screen  BW and BWY and BWR */
#define USE_420 defined ( USE_420_BWR ) || defined ( USE_420_BWY ) || \
                defined ( USE_420_BW )
/* 5.83 inch screen  BW and BWY and BWR */
#if defined (USE_583_BWR) || defined (USE_583_BWY)/* preprocess the code to  merge the three colors */
// #undef USE_583_BWR
// #define USE_583_BWY
#define USE_583_THREE_COLORS
#endif
#define USE_583 defined ( USE_583_BW ) || defined (USE_583_THREE_COLORS)
/* 7.5 inch screen  BW and BWY and BWR */
#define USE_750 defined ( USE_750_BW ) || defined ( USE_750_BWR ) || \
                defined ( USE_750_BWY )
/* Special initialization screen */
#define SPECIAL_SCREEN   defined ( USE_154_BW_GREEN ) || defined ( USE_290_BW ) || \
                         defined ( USE_154_BW_BLUE )

/* Ordinary initialization screen */ 
#define ORDINARY_SCREEN  defined ( USE_260_BW  ) || defined ( USE_260_BWR ) || \
                         defined ( USE_270_BW  ) || defined ( USE_270_BWR ) || \
                         defined ( USE_290_BWSoft ) || defined ( USE_290_BWY ) || \
                         defined ( USE_290_BWR ) || defined ( USE_420_BW  ) || \
                         defined ( USE_420_BWR ) || defined ( USE_420_BWY ) || \
                         defined ( USE_154_BWY ) || defined ( USE_154_BWR_152 ) || \
                         defined (  USE_213_BWR  ) || defined ( USE_213_BWY ) || \
                         defined ( USE_213_BWSoft ) || defined ( USE_213_BW ) || \
                         defined ( USE_583_BW ) ||  defined ( USE_583_THREE_COLORS ) || \
                         defined ( USE_750_BW ) ||  defined ( USE_750_BWR ) || \
                         defined ( USE_750_BWY )

/* Select screen size */
#if defined ( USE_154_BW_GREEN ) ||  defined ( USE_154_BW_BLUE )
/* Display resolution */
#define EPD_WIDTH       200
#define EPD_HEIGHT      200

#elif USE_154
/* Display resolution */
#define EPD_WIDTH       152
#define EPD_HEIGHT      152

#elif USE_213
/* Display resolution */
#define EPD_WIDTH       104
#define EPD_HEIGHT      212

#elif USE_260
/* Display resolution */
#define EPD_WIDTH       152
#define EPD_HEIGHT      296

#elif USE_270
/* Display resolution */
#define EPD_WIDTH       176
#define EPD_HEIGHT      264

#elif defined ( USE_290_BW )
/* Display resolution */
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

#elif USE_290
/* Display resolution */
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

#elif USE_420
/* Display resolution */
#define EPD_WIDTH       300
#define EPD_HEIGHT      400

#elif USE_583
/* Display resolution */
#define EPD_WIDTH       448
#define EPD_HEIGHT      600
#define COUNTER			67200	/* Number of bytes required */

#elif USE_750
/* Display resolution */
#define EPD_WIDTH       384
#define EPD_HEIGHT      640
#define COUNTER		   	61440	/* Number of bytes required */

#endif

#if SPECIAL_SCREEN
/* EPD commands */
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

#elif ORDINARY_SCREEN
/* EPD commands */
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13	/* 583 #define IMAGE_PROCESS_COMMAND */
#define PARTIAL_DATA_START_TRANSMISSION_1		   	0x14 	/* Only 270 screen */
#define PARTIAL_DATA_START_TRANSMISSION_2		   	0x15 	/* Only 270 screen */
#define PARTIAL_DISPLAY_REFRESH				        0X16	/* Only 270 screen */
#define VCOM_LUT                                    0x20
#define W2W_LUT                                     0x21
#define B2W_LUT                                     0x22
#define W2B_LUT                                     0x23
#define B2B_LUT                                     0x24
#define LUT_RED_0                                   0X25
#define LUT_RED_1                                   0X26
#define LUT_RED_2                                   0x27
#define LUT_RED_3                                   0x28
#define LUT_XON                                     0x29
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_CALIBRATION              0x40
#define TEMPERATURE_SENSOR_SELECTION                0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define RESOLUTION_SETTING                          0x61
#define SOURCE_GATE_START_SETTING			    	0x62    /* Only 270 screen */
#define GSST_SETTING                                0X65    /*420 and 583 screen*/
#define REVISION_SETTING                            0X70	/* Only 583 screen */
#define GET_STATUS                                  0x71
#define AUTO_MEASURE_VCOM                           0x80
#define READ_VCOM_VALUE                             0x81
#define VCM_DC_SETTING                              0x82
#define PARTIAL_WINDOW                              0x90	/* 260 and 290 */ 
#define PARTIAL_IN                                  0x91	/* 260 and 290 */ 
#define PARTIAL_OUT                                 0x92	/* 260 and 290 */ 
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAM                              0xA1
#define READ_OTP_DATA                               0xA2
#define POWER_SAVING                                0xE3	/* Only 260 screen */

#endif

extern const unsigned char lut_full_update[];
extern const unsigned char lut_partial_update[];

class Epd : EpdIf {
public:
    Epd();
    ~Epd();
    int  Init(const unsigned char* lut);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    void Reset(void);
public:
    /*2.9 inch white and black. Used for partial display*/
    void SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l);
    void SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l);
    void SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l);
    void SetFrameMemory(
        const unsigned char* image_buffer,
        int x,
        int y,
        int image_width,
        int image_height
    );
    /*1.54 green lable, 1.54 blue lable, 2.9 inch white and black  */
    void SetFrameMemory(const unsigned char* image_buffer);
    void ClearFrameMemory(unsigned char color);
    void DisplayFrame(void);

    void Sleep(void);
public:
    /* Screens of all sizes except those mentioned above */
    void DisplayFrame(const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red);
    void DisplayFrame(const unsigned char* frame_buffer_black);
    void ClearFrame(void);
    
    // void RefreshPartial(int x, int y, int w, int l);
private:
/*2.9 inch white and black*/
    void SetMemoryArea(int x_start, int y_start, int x_end, int y_end);
    void SetMemoryPointer(int x, int y);

    unsigned long width;
    unsigned long height;

   // friend class EPDClient;
private:
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int reset_pin;
    unsigned int busy_pin;
    const unsigned char* lut;
    void setPins(unsigned int ss,unsigned int reset);
    void setSPIFrequency(uint32_t frequency);
    void SetLut(void);
    void SetLut(const unsigned char* lut);
// #if USE_154
    void SetLutBw(void);
    void SetLutRed(void);
// #endif
};
extern Epd epd;
#endif
/* END OF FILE */


