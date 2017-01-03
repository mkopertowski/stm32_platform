#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include <io.h>

#include "FreeRTOS.h"
#include "timers.h"

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define LED_PORT_NUMBER               (2)
#define LED_PIN_NUMBER                (13)

#define IO_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define IO_PIN_MASK(_N)              (1 << (_N))
#define IO_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))

#define BUTTON_PORT_NUMBER            (0)
#define BUTTON_PIN_NUMBER             (4)

#define BUTTON_LONG_PRESS_TIMEOUT     (3000)

typedef struct {
    io_button_pressed_cb_t button_state_listener;
    io_module_hit_cb_t module_hit_listener;
    TimerHandle_t button_timer;
} ctx_t;

static ctx_t ctx = {0};

static void io_init_led(void)
{
    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(IO_RCC_MASKx(LED_PORT_NUMBER), ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure pin in output push/pull mode
    GPIO_InitStructure.GPIO_Pin = IO_PIN_MASK(LED_PIN_NUMBER);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(IO_GPIOx(LED_PORT_NUMBER), &GPIO_InitStructure);

    // Start with led turned off
    io_set_led_state(false);
}

static void button_press(TimerHandle_t xTimer)
{
    if(ctx.button_state_listener) {
        ctx.button_state_listener(IO_BUTTON_LONG_PRESS);
    }
}

static void io_init_button(void)
{
    ctx.button_timer = xTimerCreate("bt740_tim",pdMS_TO_TICKS(BUTTON_LONG_PRESS_TIMEOUT),false,(void *)&ctx ,button_press);

    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(IO_RCC_MASKx(BUTTON_PORT_NUMBER), ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Configure pin in output push/pull mode
    GPIO_InitStructure.GPIO_Pin = IO_PIN_MASK(BUTTON_PIN_NUMBER);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IO_GPIOx(LED_PORT_NUMBER), &GPIO_InitStructure);

    NVIC_EnableIRQ(EXTI4_IRQn);

    /* Tell system that you will use PA4 for EXTI_Line4 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);

    EXTI_InitTypeDef EXTI_InitStruct;
    /* PA4 is connected to EXTI_Line4 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
}

void EXTI4_IRQHandler(void)
{
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        /* Do your stuff when PA4 is changed */
        if(xTimerIsTimerActive(ctx.button_timer)) {
            xTimerStop(ctx.button_timer, 0 );
            if(ctx.button_state_listener) {
                ctx.button_state_listener(IO_BUTTON_SHORT_PRESS);
            }
        } else {
            xTimerStart(ctx.button_timer, 0 );
        }

        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

static void io_init_avalanche_beacon(void)
{

}

static void io_init_module_hit_sensor(void)
{

}

void io_init(void)
{
    /* init LED */
    io_init_led();

    /* init button */
    io_init_button();

    /* avalnache beacon */
    io_init_avalanche_beacon();

    /* module hit detection */
    io_init_module_hit_sensor();
}

void io_set_led_state(bool state)
{
    if(state) {
        GPIO_ResetBits(IO_GPIOx(LED_PORT_NUMBER),IO_PIN_MASK(LED_PIN_NUMBER));
    } else {
        GPIO_SetBits(IO_GPIOx(LED_PORT_NUMBER),IO_PIN_MASK(LED_PIN_NUMBER));
    }
}

void io_button_register_listener(io_button_pressed_cb_t cb)
{
    ctx.button_state_listener = cb;
}

void io_set_avalnache_beacon_state(bool state)
{

}

void io_module_hit_register_listener(io_module_hit_cb_t cb)
{
    ctx.module_hit_listener = cb;
}

