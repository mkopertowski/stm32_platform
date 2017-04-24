#include <stdio.h>
#include <stdlib.h>

#include <platform/io.h>
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "FreeRTOS.h"
#include "timers.h"

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define LED_PORT_NUMBER               (2)
#define LED_PIN_NUMBER                (13)

#define IO_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define IO_PIN_MASK(_N)              (1 << (_N))
#define IO_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))

#define LEFT_BUTTON_PORT_NUMBER            (0)
#define LEFT_BUTTON_PIN_NUMBER             (4)

#define RIGHT_BUTTON_PORT_NUMBER            (0)
#define RIGHT_BUTTON_PIN_NUMBER             (3)

#define BUTTON_LONG_PRESS_TIMEOUT     (3000)

typedef struct {
    io_button_pressed_cb_t button_state_listener;
    io_module_hit_cb_t module_hit_listener;

    TimerHandle_t left_button_timer;
    io_button_pressed_cb_t left_button_state_listener;

    TimerHandle_t right_button_timer;
    io_button_pressed_cb_t right_button_state_listener;
} ctx_t;

static ctx_t ctx = {0};

static void io_init_led(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(IO_RCC_MASKx(LED_PORT_NUMBER), ENABLE);

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

static void io_init_left_button(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    ctx.left_button_timer = xTimerCreate("left_button_tim",pdMS_TO_TICKS(BUTTON_LONG_PRESS_TIMEOUT),false,(void *)&ctx ,button_press);

    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(IO_RCC_MASKx(LEFT_BUTTON_PORT_NUMBER), ENABLE);

    // Configure pin in output push/pull mode
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = IO_PIN_MASK(LEFT_BUTTON_PIN_NUMBER);
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IO_GPIOx(LEFT_BUTTON_PORT_NUMBER), &gpio);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    NVIC_EnableIRQ(EXTI4_IRQn);

    EXTI_StructInit(&exti);
    /* PA4 is connected to EXTI_Line4 */
    exti.EXTI_Line = EXTI_Line4;
    /* Enable interrupt */
    exti.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&exti);

    /* Tell system that you will use PA4 for EXTI_Line4 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);

    /* interrupt init */
    nvic.NVIC_IRQChannel = EXTI4_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic.NVIC_IRQChannelSubPriority = 0x00;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

static void io_init_right_button(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    ctx.right_button_timer = xTimerCreate("right_button_tim",pdMS_TO_TICKS(BUTTON_LONG_PRESS_TIMEOUT),false,(void *)&ctx ,button_press);

    // Enable GPIO Peripheral clock
    RCC_APB2PeriphClockCmd(IO_RCC_MASKx(RIGHT_BUTTON_PORT_NUMBER), ENABLE);

    // Configure pin in output push/pull mode
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = IO_PIN_MASK(RIGHT_BUTTON_PIN_NUMBER);
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IO_GPIOx(RIGHT_BUTTON_PORT_NUMBER), &gpio);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    NVIC_EnableIRQ(EXTI3_IRQn);

    EXTI_StructInit(&exti);
    /* PA4 is connected to EXTI_Line3 */
    exti.EXTI_Line = EXTI_Line3;
    /* Enable interrupt */
    exti.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&exti);

    /* Tell system that you will use PA3 for EXTI_Line3 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource3);

    /* interrupt init */
    nvic.NVIC_IRQChannel = EXTI3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic.NVIC_IRQChannelSubPriority = 0x00;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void EXTI4_IRQHandler(void)
{
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        /* Do your stuff when PA4 is changed */
        if(xTimerIsTimerActive(ctx.left_button_timer)) {
            xTimerStop(ctx.left_button_timer, 0 );
            if(ctx.left_button_state_listener) {
                ctx.left_button_state_listener(IO_BUTTON_SHORT_PRESS);
            }
        } else {
            xTimerStart(ctx.left_button_timer, 0 );
        }

        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI3_IRQHandler(void)
{
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
        /* Do your stuff when PA4 is changed */
        if(xTimerIsTimerActive(ctx.right_button_timer)) {
            xTimerStop(ctx.right_button_timer, 0 );
            if(ctx.right_button_state_listener) {
                ctx.right_button_state_listener(IO_BUTTON_SHORT_PRESS);
            }
        } else {
            xTimerStart(ctx.right_button_timer, 0 );
        }

        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void io_init(void)
{
    /* init LED */
    io_init_led();

    /* init buttons */
    io_init_left_button();
    io_init_right_button();
}

void io_set_led_state(bool state)
{
    if(state) {
        GPIO_ResetBits(IO_GPIOx(LED_PORT_NUMBER),IO_PIN_MASK(LED_PIN_NUMBER));
    } else {
        GPIO_SetBits(IO_GPIOx(LED_PORT_NUMBER),IO_PIN_MASK(LED_PIN_NUMBER));
    }
}

void io_left_button_register_listener(io_button_pressed_cb_t cb)
{
    ctx.left_button_state_listener = cb;
}

void io_right_button_register_listener(io_button_pressed_cb_t cb)
{
    ctx.right_button_state_listener = cb;
}

void io_set_headup_led(uint8_t color, uint8_t state)
{

}
