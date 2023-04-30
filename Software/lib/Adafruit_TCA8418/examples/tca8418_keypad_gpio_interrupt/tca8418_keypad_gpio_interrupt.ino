
/***************************************************

  @file tca8418_keypad_gpio_interrupt.ino

  This is an example for the Adafruit TCA8418 Keypad Matrix / GPIO Expander Breakout

  Designed specifically to work with the Adafruit TCA8418 Keypad Matrix
  ----> https://www.adafruit.com/products/XXXX

  These Keypad Matrix use I2C to communicate, 2 pins are required to
  interface.
  The Keypad Matrix has an interrupt pin to provide fast detection
  of changes.

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

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(3, 3);

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
  //  CHECKING THE STAT REGISTER
  static uint32_t last = 0;
  if (millis() - last > 1000)
  {
    last = millis();
    int x = keypad.readRegister(TCA8418_REG_INT_STAT);
    Serial.print("STAT:\t");
    Serial.print(x, HEX);
    Serial.print("\t");
    Serial.println(keypad.available(), HEX);
  }

  //  IRQ TO HANDLE?
  if (TCA8418_event == true)
  {
    //  CHECK WHICH INTERRUPTS TO HANDLE
    int intStat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if (intStat & 0x02)
    {
      //  reading the registers is mandatory to clear IRQ flag
      //  can also be used to find the GPIO changed
      //  as these registers are a bitmap of the gpio pins.
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_1);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_2);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_3);
      //  clear GPIO IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 2);
    }

    if (intStat & 0x01)
    {
      //  datasheet page 16 - Table 2
      int keyCode = keypad.getEvent();
      if (keyCode & 0x80) Serial.print("PRESS\t ");
      else Serial.print("RELEASE\t ");
      //  map keyCode to GPIO nr.
      keyCode &= 0x7F;

      if (keyCode > 96)  //  GPIO
      {
        //  process  gpio
        keyCode -= 97;
        Serial.print(keyCode);
        Serial.println();
      }
      else
      {
        //  process  matrix
        keyCode--;
        Serial.print(keyCode / 10);
        Serial.print("\t ");
        Serial.print(keyCode % 10);
        Serial.println();
      }

      //  clear the EVENT IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
    }

    //  check pending events
    int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if ((intstat & 0x03) == 0) TCA8418_event = false;

  }
  // other code here
  delay(100);
}
