#include <stdio.h>

#include "hal/hal.h"
#include "vgmdata.h"
#include "vgm_int.h"

unsigned long long total_ticks;

int main(int argc, char **argv)
{
    int i, j;
    unsigned long long last_ticks;
    unsigned long long next_ticks;
    player_state_t player_state;
    vgm_stream_t *data_stream;
    char line[80];
    char *header = "Chn|AMP|VI|EG|KSR|dbScale|dbTotal|Attck|Decay|Sustn|Rel|Frq|Key|Oct|Fdbk|Wvfrm";
    for(i=0; header[i] != '\0'; i++)
    {
        if (header[i] == '|')
        {
            for(j=1; j<25; j++)
            header[i] = 186;
        }
        line[i] = (char)205;
    }
    line[i] = '\0';

    // Configure vgm interpreter
    if(vgm_interpreter_init() == 0)
    {
        printf("Error: unable to initialize vgm interpreter\n");
        return -1;
    }

    // Configure system timer
    timer_configure(44100);

    // Initialize video mode
    vga_set_mode(VGA_80x25_16_COLOR_TEXT_MODE);
    vga_text_disable_cursor();
    vga_clear_screen(VGA_TEXT_COLOR_LIGHT_GRAY);
    vga_paint_rect(
        ' ',
        VGA_TEXT_COLOR_BLACK,
        VGA_TEXT_COLOR_BLUE,
        1, 1,
        78, 23
    );
    vga_write_text(
        "DOS VGM Player",
        VGA_TEXT_COLOR_BLUE,
        VGA_TEXT_COLOR_LIGHT_GRAY,
        33, 0
    );
    vga_write_text(
        "https://github.com/ryanmid/adlib-vgm-player",
        VGA_TEXT_COLOR_BLUE,
        VGA_TEXT_COLOR_LIGHT_GRAY,
        19, 24
    );
    vga_write_text(
        header,
        VGA_TEXT_COLOR_LIGHT_GRAY | VGA_TEXT_COLOR_LIGHT_BIT,
        VGA_TEXT_COLOR_BLUE,
        1, 1
    );
    vga_write_text(
        line,
        VGA_TEXT_COLOR_LIGHT_GRAY | VGA_TEXT_COLOR_LIGHT_BIT,
        VGA_TEXT_COLOR_BLUE,
        1, 2
    );

    // Open data stream
    data_stream = vgm_open_stream(argv[1]);

    last_ticks = hal_ticks;
    total_ticks = 0;
    player_state.is_playing = 1;
    player_state.delay = 0;
    while(player_state.is_playing)
    {
        // Do nothing if it isn't yet time for another tick
        next_ticks = hal_ticks;
        if (next_ticks == last_ticks)
           continue;

        last_ticks = next_ticks;
        total_ticks++;

        // If the stream is in the middle of a delay then do nothing
        // during this tick
        if (player_state.delay > 0)
        {
            --player_state.delay;
            continue;
        }

        // Advance the player to the next tick in the stream
        player_state = vgm_interpreter_tick(data_stream);

        // Check the player state to see if the player needs to stop
        if (player_state.is_error)
        {
            player_state.is_playing = 0;
        }
        else if (player_state.is_playing == 0)
        {
            player_state.is_playing = 0;
        }
    }

    adlib_init();
    timer_restore();
    vgm_close_stream(data_stream);
  
    // Restore video mode
    //vga_set_mode(VGA_80x25_16_COLOR_TEXT_MODE);
    vga_text_enable_cursor();

    return 0;
}
