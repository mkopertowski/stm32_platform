#include "stm32f10x.h"
#include <io.h>

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define LED_PORT_NUMBER               (2)
#define LED_PIN_NUMBER                (13)

#define LED_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define LED_PIN_MASK(_N)              (1 << (_N))
#define LED_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))

void io_init(void)
{
    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(LED_RCC_MASKx(LED_PORT_NUMBER), ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure pin in output push/pull mode
    GPIO_InitStructure.GPIO_Pin = LED_PIN_MASK(LED_PIN_NUMBER);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_GPIOx(LED_PORT_NUMBER), &GPIO_InitStructure);

    // Start with led turned off
    io_set_led_state(false);
}

io_set_led_state(bool state)
{
    if(state) {
        GPIO_ResetBits(LED_GPIOx(LED_PORT_NUMBER),LED_PIN_MASK(LED_PIN_NUMBER));
    } else {
        GPIO_SetBits(LED_GPIOx(LED_PORT_NUMBER),LED_PIN_MASK(LED_PIN_NUMBER));
    }
}
