#ifndef LED_RENDERER
#define LED_RENDERER



void init_led(int num_leds, unsigned **strip_buffer);

void render_to_led();

void clear_led(void);

void shutdown_led();



#endif // LED_RENDERER
