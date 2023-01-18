#ifndef __EPDBASE_H__
#define __EPDBASE_H__
#include <Arduino.h>
#include <SPI.h>

class EpdBase {
  protected:
    uint8_t     dc_pin;
    uint8_t     cs_pin;
    uint8_t     reset_pin;
    uint8_t     busy_pin;
    uint32_t    freq;
    uint8_t     spi_num;
    uint8_t     clk_pin;

  public:
    EpdBase(uint8_t _rst, uint8_t _dc, uint8_t _cs,  uint8_t _busy, int8_t _clk=-1, uint8_t _spi_num = 0 ,uint32_t _freq = 2000000) {
      this->dc_pin = _dc;
      this->cs_pin = _cs;
      this->reset_pin = _rst;
      this->busy_pin = _busy;
      this->freq = _freq;
      this->spi_num = _spi_num;
      this->clk_pin = _clk;
    }
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    void Reset(void);

  protected:
    int  IfInit(void);
    void DigitalWrite(int pin, int value); 
    int  DigitalRead(int pin);
    void DelayMs(unsigned int delaytime);
    void SpiTransfer(unsigned char data);
};
#endif