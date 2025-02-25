#include "gb_joypad.h"
uint8_t joypad_register=0xCF;
uint8_t joypad_dpad_values=0xCF;
uint8_t joypad_button_values=0xCF;
uint8_t gb_joypad_is_d_pad_selected()
{
    return !(joypad_register&0b00010000);
}
uint8_t gb_joypad_are_buttons_selected()
{
    return !(joypad_register&0b00100000);
}

uint8_t* gb_joypad_get_joypad_dpad_values()
{
    return &joypad_dpad_values;
}
uint8_t* gb_joypad_get_joypad_button_values()
{
    return &joypad_button_values;
}
uint8_t* gb_joypad_get_joypad_register()
{
    if(gb_joypad_are_buttons_selected())
    {
        return gb_joypad_get_joypad_button_values();
    }
    if(gb_joypad_is_d_pad_selected())
    {
        return gb_joypad_get_joypad_dpad_values();
    }
}
void gb_joypad_set_selection(uint8_t value)
{
    if(value&0b00010000)
    {
        joypad_register=value&0x10;
    }
    else if (value&0b00100000)
    {
        joypad_register=value&0x20;
    }
    
}