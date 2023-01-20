#include "Tca8418Keyboard.h"

Tca8418Keyboard::Tca8418Keyboard(uint8_t numrows, uint8_t numcols) {
  tca8418_address = TCA8418_I2C_ADDRESS;
  num_rows = numrows;
  num_cols = numcols;

  delta_micros = 0;
  last_update_micros = 0;
  this_update_micros = 0;

  pressed_list = new PressedKey[KEY_EVENT_LIST_SIZE];
  released_list = new ReleasedKey[KEY_EVENT_LIST_SIZE];
  matrix_state = new uint16_t[num_rows];
  matrix_state_prev = new uint16_t[num_rows];

}

Tca8418Keyboard::~Tca8418Keyboard(void) {
  delete [] matrix_state_prev;
  delete [] matrix_state;
  delete [] pressed_list;
  delete [] released_list;
}

void Tca8418Keyboard::begin(void) {
  Wire.begin();

  /*
    | ADDRESS | REGISTER NAME | REGISTER DESCRIPTION | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0 |
    |---------+---------------+----------------------+------+------+------+------+------+------+------+------|
    |    0x1D | KP_GPIO1      | Keypad/GPIO Select 1 | ROW7 | ROW6 | ROW5 | ROW4 | ROW3 | ROW2 | ROW1 | ROW0 |
    |    0x1E | KP_GPIO2      | Keypad/GPIO Select 2 | COL7 | COL6 | COL5 | COL4 | COL3 | COL2 | COL1 | COL0 |
    |    0x1F | KP_GPIO3      | Keypad/GPIO Select 3 | N/A  | N/A  | N/A  | N/A  | N/A  | N/A  | COL9 | COL8 |
  */

  // everything enabled in key scan mode
  uint8_t enabled_rows = 0x3F;
  uint16_t enabled_cols = 0x3FF;

  write(REGISTER_KP_GPIO1, enabled_rows);
  write(REGISTER_KP_GPIO2, (uint8_t)(0xFF & enabled_cols));
  write(REGISTER_KP_GPIO3, (uint8_t)(0x03 & (enabled_cols >> 8)));

  /*
    BIT: NAME

    7: AI
    Auto-increment for read and write operations; See below table for more information
    0 = disabled
    1 = enabled

    6: GPI_E_CFG
    GPI event mode configuration
    0 = GPI events are tracked when keypad is locked
    1 = GPI events are not tracked when keypad is locked

    5: OVR_FLOW_M
    Overflow mode
    0 = disabled; Overflow data is lost
    1 = enabled; Overflow data shifts with last event pushing first event out

    4: INT_CFG
    Interrupt configuration
    0 = processor interrupt remains asserted (or low) if host tries to clear interrupt while there is
    still a pending key press, key release or GPI interrupt
    1 = processor interrupt is deasserted for 50 μs and reassert with pending interrupts

    3: OVR_FLOW_IEN
    Overflow interrupt enable
    0 = disabled; INT is not asserted if the FIFO overflows
    1 = enabled; INT becomes asserted if the FIFO overflows

    2: K_LCK_IEN
    Keypad lock interrupt enable
    0 = disabled; INT is not asserted after a correct unlock key sequence
    1 = enabled; INT becomes asserted after a correct unlock key sequence

    1: GPI_IEN
    GPI interrupt enable to host processor
    0 = disabled; INT is not asserted for a change on a GPI
    1 = enabled; INT becomes asserted for a change on a GPI

    0: KE_IEN
    Key events interrupt enable to host processor
    0 = disabled; INT is not asserted when a key event occurs
    1 = enabled; INT becomes asserted when a key event occurs
   */

  // 10111001 xB9 -- fifo overflow enabled
  // 10011001 x99 -- fifo overflow disabled
  write(REGISTER_CFG, 0x99);

  clear_released_list();
  clear_pressed_list();

  delayMicroseconds(100);
}

void Tca8418Keyboard::write(uint8_t register_address, uint8_t data) {
  Wire.beginTransmission(tca8418_address);
  Wire.write((uint8_t) register_address);
  Wire.write((uint8_t) data);
  Wire.endTransmission();
}

bool Tca8418Keyboard::read(uint8_t register_address, uint8_t *data) {
  Wire.beginTransmission(tca8418_address);
  Wire.write(register_address);
  Wire.endTransmission();

  uint8_t timeout=0;
  Wire.requestFrom(tca8418_address, (uint8_t) 0x01);

  while(Wire.available() < 1) {
    timeout++;
    if(timeout > 100) {
      return false;
    }
    delayMicroseconds(1);
  }
  *data = Wire.read();
  return true;
}

bool Tca8418Keyboard::update() {
  last_update_micros = this_update_micros;
  uint8_t key_code, key_down, key_event, key_row, key_col;

  key_event = get_key_event();
  // TODO: read gpio R7/R6 status? 0x14 bits 7&6
  // read(0x14, &new_keycode)

  this_update_micros = micros();
  delta_micros = this_update_micros - last_update_micros;

  // if there is a new event
  if (key_event > 0) {
    key_code = key_event & 0x7F;
    key_down = (key_event & 0x80) >> 7;
    key_row = key_code / num_cols;
    key_col = key_code % num_cols;

    // Serial.println(key_code);
    // Serial.println(key_down); 

    // always clear the released list
    clear_released_list();

    if (key_down) {
      add_pressed_key(key_row, key_col);
      // TODO reject ghosts (assume multiple key presses with the same hold time are ghosts.)

    }
    else {
      add_released_key(key_row, key_col);
      remove_pressed_key(key_row, key_col);
    }

    return true;
  }

  // increment hold times for pressed keys
  for (int i=0; i<pressed_key_count; i++) {
    pressed_list[i].hold_time += delta_micros;
  }

  return false;
}


void Tca8418Keyboard::add_pressed_key(uint8_t row, uint8_t col) {
  if (pressed_key_count >= KEY_EVENT_LIST_SIZE)
    return;

  pressed_list[pressed_key_count].row = row;
  pressed_list[pressed_key_count].col = col;
  pressed_list[pressed_key_count].hold_time = 0;
  pressed_key_count++;
}

void Tca8418Keyboard::add_released_key(uint8_t row, uint8_t col) {
  if (released_key_count >= KEY_EVENT_LIST_SIZE)
    return;

  released_key_count++;
  released_list[0].row = row;
  released_list[0].col = col;
}


void Tca8418Keyboard::remove_pressed_key(uint8_t row, uint8_t col) {
  if (pressed_key_count == 0)
    return;

  // delete the pressed key
  for (int i=0; i<pressed_key_count; i++) {
    if (pressed_list[i].row == row &&
        pressed_list[i].col == col) {
      // shift remaining keys left one index
      for (int j=i; i<pressed_key_count; j++) {
        if (j == KEY_EVENT_LIST_SIZE - 1)
          break;
        pressed_list[j].row = pressed_list[j+1].row;
        pressed_list[j].col = pressed_list[j+1].col;
        pressed_list[j].hold_time = pressed_list[j+1].hold_time;
      }
      break;
    }
  }
  pressed_key_count--;
}

void Tca8418Keyboard::clear_pressed_list() {
  for (int i=0; i<KEY_EVENT_LIST_SIZE; i++) {
    pressed_list[i].row = 255;
    pressed_list[i].col = 255;
  }
  pressed_key_count = 0;
}

void Tca8418Keyboard::clear_released_list() {
  for (int i=0; i<KEY_EVENT_LIST_SIZE; i++) {
    released_list[i].row = 255;
    released_list[i].col = 255;
  }
  released_key_count = 0;
}

uint8_t Tca8418Keyboard::get_key_event() {
  uint8_t new_keycode = 0;

  read(REGISTER_KEY_EVENT_A, &new_keycode);
  return(new_keycode);
}
