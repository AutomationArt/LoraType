#ifndef TCA8418KEYBOARD_H
#define TCA8418KEYBOARD_H

#include <Arduino.h>
#include <Wire.h>

#define TCA8418_I2C_ADDRESS 0x34

#define KEY_EVENT_LIST_SIZE 10

#define REGISTER_CFG 0x01
#define REGISTER_KP_GPIO1 0x1D
#define REGISTER_KP_GPIO2 0x1E
#define REGISTER_KP_GPIO3 0x1F

#define REGISTER_KEY_EVENT_A 0x04
#define REGISTER_KEY_EVENT_B 0x05
#define REGISTER_KEY_EVENT_C 0x06
#define REGISTER_KEY_EVENT_D 0x07
#define REGISTER_KEY_EVENT_E 0x08
#define REGISTER_KEY_EVENT_F 0x09
#define REGISTER_KEY_EVENT_G 0x0A
#define REGISTER_KEY_EVENT_H 0x0B
#define REGISTER_KEY_EVENT_I 0x0C
#define REGISTER_KEY_EVENT_J 0x0D

struct PressedKey {
  uint8_t row;
  uint8_t col;
  uint32_t hold_time;
};

struct ReleasedKey {
  uint8_t row;
  uint8_t col;
};

class Tca8418Keyboard {
public:
  Tca8418Keyboard(uint8_t num_rows, uint8_t num_cols);
	~Tca8418Keyboard();

  uint8_t num_rows;
  uint8_t num_cols;

  uint16_t *matrix_state;
  uint16_t *matrix_state_prev;

  uint32_t delta_micros;

  PressedKey *pressed_list;
  ReleasedKey *released_list;
  uint8_t pressed_key_count;
  uint8_t released_key_count;

  void begin();
  bool update();
  uint8_t get_key_event();
  bool button_pressed(uint8_t row, uint8_t button_bit_position);
  bool button_released(uint8_t row, uint8_t button_bit_position);
  bool button_held(uint8_t row, uint8_t button_bit_position);

 private:
  uint8_t tca8418_address;
  uint32_t last_update_micros;
  uint32_t this_update_micros;

  uint8_t new_pressed_keys_count;

  void clear_released_list();
  void clear_pressed_list();
  void add_pressed_key(uint8_t row, uint8_t col);
  void add_released_key(uint8_t row, uint8_t col);
  void remove_pressed_key(uint8_t row, uint8_t col);
  void write(uint8_t register_address, uint8_t data);
  bool read(uint8_t register_address, uint8_t *data);
};
#endif
