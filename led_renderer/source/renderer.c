/*
 * newtest.c
 *
 * Copyright (c) 2014 Jeremy Garff <jer @ jers.net>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     1.  Redistributions of source code must retain the above copyright notice, this list of
 *         conditions and the following disclaimer.
 *     2.  Redistributions in binary form must reproduce the above copyright notice, this list
 *         of conditions and the following disclaimer in the documentation and/or other materials
 *         provided with the distribution.
 *     3.  Neither the name of the owner nor the names of its contributors may be used to endorse
 *         or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


// static char VERSION[] = "XX.YY.ZZ";

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

#include "renderer.h"

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"

#include "ws2811.h"







typedef struct LEDStripData
{
  ws2811_t info;
  int num_leds;
  ws2811_led_t *led_values;
} LEDStripData;





// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
//#define STRIP_TYPE            WS2811_STRIP_RGB    // WS2812/SK6812RGB integrated chip+leds
#define STRIP_TYPE              WS2811_STRIP_GBR    // WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW    // SK6812RGBW (NOT SK6812RGB)

//#define DEFAULT_LED_COUNT               (4)




// Global for now
struct LEDStripData;
static LEDStripData *global_strip;



void init_led(int num_leds, unsigned **strip_buffer)
{
  global_strip = (LEDStripData *)malloc(sizeof(LEDStripData));

  ws2811_t info =
  {
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
      [0] =
      {
        .gpionum = GPIO_PIN,
        .count = num_leds,
        .invert = 0,
        .brightness = 255,
        .strip_type = STRIP_TYPE,
      },
    [1] =
      {
        .gpionum = 0,
        .count = 0,
        .invert = 0,
        .brightness = 0,
      },
    },
  };
  global_strip->info = info;

  global_strip->num_leds = num_leds;

  int result = ws2811_init(&(global_strip->info));
  if(result != WS2811_SUCCESS)
  {
    fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(result));
  }

  global_strip->led_values = global_strip->info.channel[0].leds;
  *strip_buffer = global_strip->led_values;
}

void render_to_led()
{
  int result = ws2811_render(&(global_strip->info));
  if(result != WS2811_SUCCESS)
  {
    fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(result));
  }
}

void clear_led(void)
{
  memset(global_strip->led_values, 0, global_strip->num_leds * sizeof(*(global_strip->led_values)));
}


void shutdown_led()
{
  // Render an empty led strip
  clear_led();
  ws2811_render(&global_strip->info); 

  ws2811_fini(&global_strip->info);

  free(global_strip);
}

