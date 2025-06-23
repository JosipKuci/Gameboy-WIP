#include "gb_cartridge.h"
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
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
void gb_cartridge_setup_banking()
{
    for(int i=0;i<16;i++)
    {
        //First set all banks to 0
        cartridge_info.all_ram_banks[i]=0;
        if((cartridge_info.header->RAM_size==2 && i==0) || 
        (cartridge_info.header->RAM_size==3 && i<4) || 
        (cartridge_info.header->RAM_size==4 && i<16) || 
        (cartridge_info.header->RAM_size==5 && i<8))
        {
            cartridge_info.all_ram_banks[i]=malloc(0x2000); //8kB
            memset(cartridge_info.all_ram_banks[i],0,0x2000);
        }
    }
    cartridge_info.current_ram_bank=cartridge_info.all_ram_banks[0];
    cartridge_info.rom_bank_x=cartridge_info.rom_data+0x4000;
    
}

void gb_cartridge_load_battery()
{
    char filename[1024];
    sprintf(filename, "%s.battery", cartridge_info.header->title);
    FILE* file = fopen(filename, "rb");
    if(!file)
    {
        fprintf(stderr, "Failed to open battery save file: %s \n",filename);
        return;
    }
    fread(cartridge_info.current_ram_bank, 0x2000, 1, file);
    fclose(file);
}

void gb_cartridge_save_battery()
{
    char filename[1024];
    sprintf(filename, "%s.battery", cartridge_info.header->title);
    FILE* file = fopen(filename, "wb");
    if(!file)
    {
        fprintf(stderr, "Failed to open battery save file: %s \n",filename);
        return;
    }
    fwrite(cartridge_info.current_ram_bank, 0x2000, 1, file);
    fclose(file);
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
    cartridge_info.has_battery=gb_cartridge_does_cartridge_have_battery();
    cartridge_info.should_save_battery_data=false; //by default

    printf("Cartridge Loaded:\n");
    printf("\t Title    : %s\n", cartridge_info.header->title);
    printf("\t Type     : %2.2X (%s)\n", cartridge_info.header->cartridge_type, gb_get_cartridge_type_name());
    printf("\t ROM Size : %d KB\n", 32 << cartridge_info.header->ROM_size);
    printf("\t RAM Size : %2.2X\n", cartridge_info.header->RAM_size);
    printf("\t LIC Code : %2.2X (%s)\n", cartridge_info.header->old_license_code, gb_get_cartridge_license_name());
    printf("\t ROM Vers : %2.2X\n", cartridge_info.header->version_number);
    printf("Checksum: %d \n", gb_test_checksum(cartridge_info.rom_data));

    gb_cartridge_setup_banking();

    if(cartridge_info.has_battery)
    {
        gb_cartridge_load_battery();
    }
    return true;
}

uint8_t gb_cartridge_does_cartridge_need_save()
{
    return cartridge_info.should_save_battery_data;
}

uint8_t gb_cartridge_is_cartridge_mbc1()
{
    if(cartridge_info.header->cartridge_type>0 && cartridge_info.header->cartridge_type<=0x13)
    {
        return 1;
    }
    return 0;
}
//Promjeniti ovo kasnije za mbc3
uint8_t gb_cartridge_does_cartridge_have_battery()
{
    return cartridge_info.header->cartridge_type==3;
}



uint8_t gb_cartridge_read(uint16_t address)
{
    if(address < 0x4000)
    {
        return cartridge_info.rom_data[address];
    }
    if(!gb_cartridge_is_cartridge_mbc1())
    {
        return cartridge_info.rom_data[address];
    }
    if((address & 0xE000)==0xA000)//Provjeriti ovo detaljnije jos
    {
        if(!cartridge_info.is_ram_enabled)
        {
            return 0xFF;
        }
        if(!cartridge_info.current_ram_bank)
        {
            return 0xFF;
        }

        return cartridge_info.current_ram_bank[address-0xA000];

    }
    return cartridge_info.rom_bank_x[address-0x4000];

}
void gb_cartridge_write(uint8_t value, uint16_t address)
{
    if(!gb_cartridge_is_cartridge_mbc1())
    {
        return;
    }

    if(address < 0x2000)
    {
        cartridge_info.is_ram_enabled=((value&0xF)==0xA);
    }
    if((address & 0xE000)==0x2000)
    {
        //Rom bank number
        if(value ==0)
        {
            value=1;
        }
        value&=0b11111; //Only need the bottom 5 bits

        cartridge_info.rom_bank_value=value;
        cartridge_info.rom_bank_x=cartridge_info.rom_data+(cartridge_info.rom_bank_value*0x4000);
    }
    if((address & 0xE000)==0x4000)
    {
        //RAM bank number (mozd budem trebao doraditi)
        cartridge_info.ram_bank_value=value&0b11; //Only lower 2 bits
        if(cartridge_info.is_ram_banking)
        {
            if(cartridge_info.should_save_battery_data)
            {
                gb_cartridge_save_battery();
            }
            cartridge_info.current_ram_bank=cartridge_info.all_ram_banks[cartridge_info.ram_bank_value];
        }
    }
    if((address & 0xE000)==0x6000)
    {
        //Selecting the banking mode
        cartridge_info.banking_mode = value&1; //Taking just the bottom bit
        cartridge_info.is_ram_banking=cartridge_info.banking_mode;
        if(cartridge_info.is_ram_banking) //If its enabled, we change the ram bank
        {
            cartridge_info.current_ram_bank=cartridge_info.all_ram_banks[cartridge_info.ram_bank_value];

            if(cartridge_info.should_save_battery_data)
            {
                gb_cartridge_save_battery();
            }
        }

        
    }
    if((address & 0xE000)==0xA000)
    {
        if(!cartridge_info.is_ram_enabled || !cartridge_info.current_ram_bank)
        {
            return;
        }
        cartridge_info.current_ram_bank[address-0xA000]=value;

        if(cartridge_info.has_battery)
        {
            cartridge_info.should_save_battery_data=true;
        }
    }
}
