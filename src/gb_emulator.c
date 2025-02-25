#include "gb_emulator.h"
#include "gb_cpu.h"
#include "gb_dma.h"
#include "gb_interface.h"
#include "gb_timer.h"
#include "gb_cartridge.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include <pthread.h>
#include "gb_ppu.h"
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
    for(int i=1; i<=cpu_cycles;i++)
    {
        for(int j=1;j<=4;j++)
        {
            emulator_info.timer_ticks++;
            gb_timer_tick();
            gb_ppu_tick();
        }
        gb_dma_tick();
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
    gb_ppu_initialize();
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
    uint32_t previous_frame=0;
    while(!emulator_info.is_killed)
    {
        _sleep(1);
        gb_interface_handle_events();
        if(previous_frame != gb_ppu_get_info()->current_frame)
        {
            gb_ui_update_window();
        }
        previous_frame=gb_ppu_get_info()->current_frame;
    }
    return 0;
}
