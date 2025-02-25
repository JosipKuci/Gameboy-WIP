#ifndef GB_JOYPAD_H
#define GB_JOYPAD_H
#include <stdint.h>
void gb_joypad_initialize();
uint8_t gb_joypad_is_d_pad_selected();
uint8_t gb_joypad_are_buttons_selected();
void gb_joypad_set_selection(uint8_t value);
uint8_t* gb_joypad_get_joypad_register();
uint8_t* gb_joypad_get_joypad_dpad_values();
uint8_t* gb_joypad_get_joypad_button_values();
#endif