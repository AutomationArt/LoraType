#include <Adafruit_TCA8418.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>

Adafruit_TCA8418 keypad;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

#define ROWS 3
#define COLS 4

char keymap[COLS][ROWS] = {{'*', '0', '#'},
                           {'7', '8', '9'},
                           {'4', '5', '6'},
                           {'1', '2', '3'},};

void setup()
{
  Serial.begin(115200);

  display.begin(0x3C, true); // Address 0x3C default
  display.display();
  delay(1000);
  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextWrap(true);
  display.setTextColor(SH110X_WHITE);
  
  if (! keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("keypad not found, check wiring & pullups!");
    while (1);
  }

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(ROWS, COLS);

  // flush the internal buffer
  keypad.flush();

  display.clearDisplay();
  display.setCursor(0, 15);
  display.println("TCA8418 Demo");
}


void loop()
{
  
  if (keypad.available() > 0)
  {
    //  datasheet page 15 - Table 1
    int k = keypad.getEvent();
    bool pressed = k & 0x80;
    k &= 0x7F;
    k--;
    uint8_t row = k / 10;
    uint8_t col = k % 10;

    if (pressed) 
      Serial.print("PRESS\tR: ");
    else 
      Serial.print("RELEASE\tR: ");
    Serial.print(row);
    Serial.print("\tC: ");
    Serial.print(col);
    Serial.print(" - ");
    Serial.print(keymap[col][row]);
    if (pressed) {
      display.print(keymap[col][row]);
    }
    Serial.println();
  }
  display.display();
}
