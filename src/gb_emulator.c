#include "gb_emulator.h"
#include "gb_cpu.h"
#include "gb_interface.h"
#include "gb_timer.h"
#include "gb_cartridge.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include <pthread.h>
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
void gb_emulator_cycle(int cpu_cycles)
{
    int number_of_cycles=cpu_cycles*4;
    for(int i=1; i<=number_of_cycles;i++)
    {
        emulator_info.timer_ticks++;
        gb_timer_tick();
    }
    
}
gb_emulator_delay(uint32_t ms)
{
    SDL_Delay(ms);
}
void *gb_cpu_run(void *p)
{
    gb_cpu_init();
    gb_timer_init();
    emulator_info.is_running=true;
    emulator_info.is_paused=false;
    emulator_info.timer_ticks=0;
    while(emulator_info.is_running)
    {
        if(emulator_info.is_paused)
        {
            gb_emulator_delay(1);
            continue;
        }
        if (!gb_cpu_step()) {
            printf("CPU Stopped\n");
            return -3;
        }
    }
    return 0;
}
int gb_emulator_init(int argc, char **argv)
{
    gb_initialize_interface();
    
    gb_emulator_set_info(&emulator_info);
    if (!gb_load_cartridge(argv[1])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }
    pthread_t t1;

    if (pthread_create(&t1, NULL, gb_cpu_run, NULL)) {
        fprintf(stderr, "FAILED TO START MAIN CPU THREAD!\n");
        return -1;
    }
    while(!emulator_info.is_killed)
    {
        _sleep(1);
        gb_interface_handle_events();
    }
    return 0;
}
