#include "EpdBase.h"

/**
 *  @brief: basic function for sending commands
 */
void EpdBase::SendCommand(unsigned char command) {
  DigitalWrite(dc_pin, LOW);
  SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void EpdBase::SendData(unsigned char data) {
  DigitalWrite(dc_pin, HIGH);
  SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes LOW
 */
void EpdBase::WaitUntilIdle(void) {
  while(DigitalRead(busy_pin) == HIGH) {      //LOW: idle, HIGH: busy
    DelayMs(100);
  }      
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void EpdBase::Reset(void) {
  DigitalWrite(reset_pin, LOW);                //module reset    
  DelayMs(100);
  DigitalWrite(reset_pin, HIGH);
  DelayMs(100);    
}

void EpdBase::DigitalWrite(int pin, int value) {
  digitalWrite(pin, value);
}

int EpdBase::DigitalRead(int pin) {
  return digitalRead(pin);
}

void EpdBase::DelayMs(unsigned int delaytime) {
  delay(delaytime);
}

void EpdBase::SpiTransfer(unsigned char data) {
  digitalWrite(this->cs_pin, LOW);
  SPI.transfer(data);
  digitalWrite(this->cs_pin, HIGH);
}

int EpdBase::IfInit(void) {
  pinMode(this->cs_pin, OUTPUT);
  pinMode(this->reset_pin, OUTPUT);
  pinMode(this->dc_pin, OUTPUT);
  pinMode(this->busy_pin, INPUT); 
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  return 0;
}