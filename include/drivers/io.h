#ifndef IO_H_
#define IO_H_

#include <global.h>

typedef enum {
    IO_BUTTON_SHORT_PRESS,
    IO_BUTTON_LONG_PRESS,
} io_button_state_t;

typedef void (*io_button_pressed_cb_t)(io_button_state_t state);
typedef void (*io_module_hit_cb_t)(void);

void io_init(void);
void io_set_led_state(bool state);
void io_button_register_listener(io_button_pressed_cb_t cb);

void io_set_avalnache_beacon_state(bool state);
void io_module_hit_register_listener(io_module_hit_cb_t cb);

#endif // IO_H
