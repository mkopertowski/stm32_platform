#ifndef IO_H_
#define IO_H_

#include <global.h>

typedef enum {
    IO_BUTTON_SHORT_PRESS,
    IO_BUTTON_LONG_PRESS,
} io_button_state_t;

#define LED_RED_UP 1
#define LED_RED_DOWN 2
#define LED_GREEN 3

#define IO_STATE_ON_SHORT 1
#define IO_STATE_ON_LONG  2
#define IO_STATE_OFF 0
#define IO_STATE_ON 3

typedef void (*io_button_pressed_cb_t)(io_button_state_t state);
typedef void (*io_module_hit_cb_t)(void);

void io_init(void);

void io_set_led_state(bool state);
void io_left_button_register_listener(io_button_pressed_cb_t cb);
void io_right_button_register_listener(io_button_pressed_cb_t cb);

void io_set_headup_led(uint8_t color, uint8_t state);

#endif // IO_H
