#ifndef GB_CARTRIDGE_H
#define GB_CARTRIDGE_H
#include<stdint.h>
#include<stdbool.h>
/*
Each cartridge contains a header, located at the address range 0x0100—0x014F.
The cartridge header provides the following information about the game itself 
and the hardware it expects to run on
*/
struct gb_cartridge_header
{
    uint8_t Entry_point[4];
    uint8_t Nintendo_logo[0x30];

    char title[16]; //contain the title of the game in upper case ASCII.
    uint16_t new_license_code; //a two-character ASCII “licensee code” indicating the game’s publisher.
    uint8_t SGB_flag; //This byte specifies whether the game supports Super Game boy functions.
    uint8_t cartridge_type;
    uint8_t ROM_size; //byte value that indicates how much ROM is present on the cartridge.
    uint8_t RAM_size; //byte value that indicates how much RAM is present on the cartridge, if any.
    uint8_t destination_code; //this byte specifies whether this version of the game is intended to be sold in Japan or elsewhere.
    uint8_t old_license_code;//This byte is used in older (pre-SGB) cartridges to specify the game’s publisher. However, the value $33 indicates that the New licensee codes must be considered instead.
    uint8_t version_number;//Mask ROM version number
    uint8_t checksum;
    uint16_t global_checksum;//These bytes contain a 16-bit (big-endian) checksum simply computed as the sum of all the bytes of the cartridge ROM (except these two checksum bytes).
};

struct gb_cartridge_info
{
    struct gb_cartridge_header* header;
    uint32_t gb_rom_size;
    uint8_t* rom_data;

    //MBC1
    uint8_t is_ram_enabled;
    uint8_t is_ram_banking;
    uint8_t* rom_bank_x; //First rom bank
    uint8_t banking_mode;
    uint8_t rom_bank_value;
    uint8_t ram_bank_value;
    uint8_t* current_ram_bank;
    uint8_t* all_ram_banks[16];

    uint8_t has_battery;
    uint8_t should_save_battery_data;
};

static const char *ROM_TYPES[] = {
    "ROM ONLY",
    "MBC1",
    "MBC1+RAM",
    "MBC1+RAM+BATTERY",
    "0x04 ???",
    "MBC2",
    "MBC2+BATTERY",
    "0x07 ???",
    "ROM+RAM 1",
    "ROM+RAM+BATTERY 1",
    "0x0A ???",
    "MMM01",
    "MMM01+RAM",
    "MMM01+RAM+BATTERY",
    "0x0E ???",
    "MBC3+TIMER+BATTERY",
    "MBC3+TIMER+RAM+BATTERY 2",
    "MBC3",
    "MBC3+RAM 2",
    "MBC3+RAM+BATTERY 2",
    "0x14 ???",
    "0x15 ???",
    "0x16 ???",
    "0x17 ???",
    "0x18 ???",
    "MBC5",
    "MBC5+RAM",
    "MBC5+RAM+BATTERY",
    "MBC5+RUMBLE",
    "MBC5+RUMBLE+RAM",
    "MBC5+RUMBLE+RAM+BATTERY",
    "0x1F ???",
    "MBC6",
    "0x21 ???",
    "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
};

static const char *LIC_CODE[0xA5] = {
    [0x00] = "None",
    [0x01] = "Nintendo R&D1",
    [0x08] = "Capcom",
    [0x13] = "Electronic Arts",
    [0x18] = "Hudson Soft",
    [0x19] = "b-ai",
    [0x20] = "kss",
    [0x22] = "pow",
    [0x24] = "PCM Complete",
    [0x25] = "san-x",
    [0x28] = "Kemco Japan",
    [0x29] = "seta",
    [0x30] = "Viacom",
    [0x31] = "Nintendo",
    [0x32] = "Bandai",
    [0x33] = "Ocean/Acclaim",
    [0x34] = "Konami",
    [0x35] = "Hector",
    [0x37] = "Taito",
    [0x38] = "Hudson",
    [0x39] = "Banpresto",
    [0x41] = "Ubi Soft",
    [0x42] = "Atlus",
    [0x44] = "Malibu",
    [0x46] = "angel",
    [0x47] = "Bullet-Proof",
    [0x49] = "irem",
    [0x50] = "Absolute",
    [0x51] = "Acclaim",
    [0x52] = "Activision",
    [0x53] = "American sammy",
    [0x54] = "Konami",
    [0x55] = "Hi tech entertainment",
    [0x56] = "LJN",
    [0x57] = "Matchbox",
    [0x58] = "Mattel",
    [0x59] = "Milton Bradley",
    [0x60] = "Titus",
    [0x61] = "Virgin",
    [0x64] = "LucasArts",
    [0x67] = "Ocean",
    [0x69] = "Electronic Arts",
    [0x70] = "Infogrames",
    [0x71] = "Interplay",
    [0x72] = "Broderbund",
    [0x73] = "sculptured",
    [0x75] = "sci",
    [0x78] = "THQ",
    [0x79] = "Accolade",
    [0x80] = "misawa",
    [0x83] = "lozc",
    [0x86] = "Tokuma Shoten Intermedia",
    [0x87] = "Tsukuda Original",
    [0x91] = "Chunsoft",
    [0x92] = "Video system",
    [0x93] = "Ocean/Acclaim",
    [0x95] = "Varie",
    [0x96] = "Yonezawa/s’pal",
    [0x97] = "Kaneko",
    [0x99] = "Pack in soft",
    [0xA4] = "Konami (Yu-Gi-Oh!)"
};

int gb_load_cartridge(char* cartridge);
const char* gb_get_cartridge_license_name();
const char* gb_get_cartridge_type_name();
bool gb_test_checksum(uint8_t* rom_data);
uint8_t gb_cartridge_read(uint16_t address);
void gb_cartridge_write(uint8_t value,uint16_t address);
uint8_t gb_cartridge_does_cartridge_need_save();
uint8_t gb_cartridge_is_cartridge_mbc1();
uint8_t gb_cartridge_does_cartridge_have_battery();
void gb_cartridge_load_battery();
void gb_cartridge_save_battery();
#endif