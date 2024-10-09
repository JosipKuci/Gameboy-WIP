#ifndef GB_EMULATOR_H
#define GB_EMULATOR_H
#endif
#include<stdbool.h>
#include<stdint.h>
struct gb_emulator_info
{
    bool is_paused;
    bool is_running;
    uint64_t timer_ticks; 
};

int gb_emulator_init(int argc, char **argv);
struct gb_emulator_info* gb_emulator_get_info();
void gb_emulator_cycle(int cpu_cycles);
void gb_emulator_set_info(struct gb_emulator_info* emulator_info);
