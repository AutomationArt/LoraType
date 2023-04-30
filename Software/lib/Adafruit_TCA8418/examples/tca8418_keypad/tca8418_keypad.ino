
/***************************************************

  @file tca8418_keypad.ino

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

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(8, 10);

  // flush the internal buffer
  keypad.flush();
}


void loop()
{
  if (keypad.available() > 0)
  {
    //  datasheet page 15 - Table 1
    int k = keypad.getEvent();
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
}
