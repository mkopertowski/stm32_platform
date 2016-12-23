#include "stm32f10x.h"
#include <io.h>

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define BLINK_PORT_NUMBER               (2)
#define BLINK_PIN_NUMBER                (12)
#define BLINK_ACTIVE_LOW                (1)

#define BLINK_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define BLINK_PIN_MASK(_N)              (1 << (_N))
#define BLINK_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))

void io_init(void)
{
    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(BLINK_RCC_MASKx(BLINK_PORT_NUMBER), ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure pin in output push/pull mode
    GPIO_InitStructure.GPIO_Pin = BLINK_PIN_MASK(BLINK_PIN_NUMBER);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BLINK_GPIOx(BLINK_PORT_NUMBER), &GPIO_InitStructure);

    // Start with led turned off
    io_set_led_state(false);
}

io_set_led_state(bool state)
{
    if(state) {
        GPIO_SetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),BLINK_PIN_MASK(BLINK_PIN_NUMBER));
    } else {
        GPIO_ResetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),BLINK_PIN_MASK(BLINK_PIN_NUMBER));
    }
}
