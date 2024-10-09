#include "gb_emulator.h"
#include "gb_cpu.h"
#include "gb_cartridge.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>
static struct gb_emulator_info emulator_info;
struct gb_emulator_info* gb_emulator_get_info()
{
    return &emulator_info;
}
void gb_emulator_set_info(struct gb_emulator_info* emulator_info)
{
    emulator_info->is_paused=false;
    emulator_info->is_running=true;
    emulator_info->timer_ticks=0;
}
void gb_emulator_delay(int ms)
{
    SDL_Delay(ms);
}
void gb_emulator_cycle(int cpu_cycles)
{
    //Not implemented
}
int gb_emulator_init(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL video initialised\n");
    TTF_Init();
    printf("TTF initialised\n");
    gb_cpu_init();
    gb_emulator_set_info(&emulator_info);
    if (!gb_load_cartridge(argv[1])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }
    while(emulator_info.is_running)
    {
        if(emulator_info.is_paused)
        {
            gb_emulator_delay(10);
            continue;
        }
        emulator_info.timer_ticks++;
        if (!gb_cpu_step()) {
            printf("CPU Stopped\n");
            return -3;
        }
    }
    return 0;
}
