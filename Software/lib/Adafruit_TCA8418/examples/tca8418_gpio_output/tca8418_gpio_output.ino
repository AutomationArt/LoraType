
/***************************************************

  @file tca8418_gpio_output.ino

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

Adafruit_TCA8418 tio;


void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(__FILE__);

  if (! tio.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("TCA8418 not found, check wiring & pullups!");
    while (1);
  }

  //  SET OUTPUT MODE
  for (int pin = 0; pin < 18; pin++)
  {
    tio.pinMode(pin, OUTPUT);
  }

  delay(1000);
}


void loop()
{
  //  do a knight light
  //  SHOW PIN STATUS
  for (int pin = 0; pin < 18; pin++)
  {
    tio.digitalWrite(pin, HIGH);
    Serial.print(pin);
    delay(250);
  }
  Serial.println();
  for (int pin = 0; pin < 18; pin++)
  {
    tio.digitalWrite(pin, LOW);
    Serial.print(pin);
    delay(250);
  }
  Serial.println();
}
