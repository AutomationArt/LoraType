
/***************************************************

  @file tca8418_keypad_interrupt.ino

  This is an example for the Adafruit TCA8418 Keypad Matrix / GPIO Expander Breakout

  Designed specifically to work with the Adafruit TCA8418 Keypad Matrix
  ----> https://www.adafruit.com/products/XXXX

  These Keypad Matrix use I2C to communicate, 2 pins are required to
  interface.
  The Keypad Matrix has an interrupt pin to provide fast detection
  of changes. This example shows the working of polling.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_TCA8418.h>

Adafruit_TCA8418 keypad;

//  typical Arduino UNO
const int IRQPIN = 3;
volatile bool TCA8418_event = false;

void TCA8418_irq()
{
  TCA8418_event = true;
}


void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(__FILE__);

  if (! keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("keypad not found, check wiring & pullups!");
    while (1);
  }

  //  configure the size of the keypad matrix.
  //  all other pins will be inputs
  keypad.matrix(8, 10);

  //  install interrupt handler
  //  going LOW is interrupt
  pinMode(IRQPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQPIN), TCA8418_irq, CHANGE);

  //  flush pending interrupts
  keypad.flush();
  //  enable interrupt mode
  keypad.enableInterrupts();
}


void loop()
{
  if (TCA8418_event == true)
  {
    //  datasheet page 15 - Table 1
    int k = keypad.getEvent();
    
    //  try to clear the IRQ flag
    //  if there are pending events it is not cleared
    keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
    int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if ((intstat & 0x01) == 0) TCA8418_event = false;

    if (k & 0x80) Serial.print("PRESS\tR: ");
    else Serial.print("RELEASE\tR: ");
    k &= 0x7F;
    k--;
    Serial.print(k / 10);
    Serial.print("\tC: ");
    Serial.print(k % 10);
    Serial.println();
  }

  // other code here
  delay(100);
}
