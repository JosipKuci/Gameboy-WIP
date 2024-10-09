#include "gb_cartridge.h"
#include<stdio.h>
#include<stdlib.h>
static struct gb_cartridge_info cartridge_info;
const char* gb_get_cartridge_license_name()
{
    if (cartridge_info.header->new_license_code <= 0xA4)
    {
        return LIC_CODE[cartridge_info.header->old_license_code];
    }
    return "UNKNOWN";
}
const char* gb_get_cartridge_type_name()
{
    if (cartridge_info.header->cartridge_type <= 0x22) {
        return ROM_TYPES[cartridge_info.header->cartridge_type];
    }

    return "UNKNOWN";
}
bool gb_test_checksum(uint8_t* rom_data)
{
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - *(rom_data+address) - 1;
    }
    if(checksum!=*(rom_data+0x014D))
    {
        return false;
    }
    return true;
}
int gb_load_cartridge(char* cartridge)
{
    FILE *file = fopen(cartridge, "r");

    if (!file) {
        printf("Failed to open: %s\n", cartridge);
        return false;
    }
    fseek(file, 0, SEEK_END);
    cartridge_info.gb_rom_size=ftell(file);
    rewind(file);
    cartridge_info.rom_data=malloc(cartridge_info.gb_rom_size);
    fread(cartridge_info.rom_data, cartridge_info.gb_rom_size, 1, file);
    fclose(file);
    cartridge_info.header=(struct gb_cartridge_header*)(cartridge_info.rom_data+0x100);
    cartridge_info.header->title[15]=0;
    printf("Cartridge Loaded:\n");
    printf("\t Title    : %s\n", cartridge_info.header->title);
    printf("\t Type     : %2.2X (%s)\n", cartridge_info.header->cartridge_type, gb_get_cartridge_type_name());
    printf("\t ROM Size : %d KB\n", 32 << cartridge_info.header->ROM_size);
    printf("\t RAM Size : %2.2X\n", cartridge_info.header->RAM_size);
    printf("\t LIC Code : %2.2X (%s)\n", cartridge_info.header->old_license_code, gb_get_cartridge_license_name());
    printf("\t ROM Vers : %2.2X\n", cartridge_info.header->version_number);
    printf("Checksum: %d \n", gb_test_checksum(cartridge_info.rom_data));
}

uint8_t gb_cartridge_read(address)
{
    return cartridge_info.rom_data[address];
}
void gb_cartridge_write(value, address)
{
    //Not implimented
}