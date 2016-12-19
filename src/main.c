//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include <debug.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Timer.h"
#include "BlinkLed.h"

#include <bt740.h>
#include <os.h>

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

 // Block for 500ms.
 const TickType_t xDelay = 500 / portTICK_PERIOD_MS;

void app_task(void *params);
void USART1Init();

int main(int argc, char* argv[])
{
    //debug_init();
    //BT740_init();
    //by default stdin/stdout are on usart2
    USART1Init();
      // turn off buffers, so IO occurs immediately
//      setvbuf(stdin, NULL, _IONBF, 0);
  //    setvbuf(stdout, NULL, _IONBF, 0);
    //  setvbuf(stderr, NULL, _IONBF, 0);
    // Send a greeting to the trace device (skipped on Release).
   //printf("H");

    // At this stage the system clock should have already been configured
    // at high speed.
    trace_printf("System clock: %u Hz\r\n", SystemCoreClock);
    //trace_putchar("a");
    //timer_start();
  
    xTaskCreate(app_task, "app_task", 512, NULL, OS_TASK_PRIORITY, NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();

    // Infinite loop
    for(;;);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    trace_puts("Stack overflow");
}

void USART1Init(void)
{
    USART_InitTypeDef usartConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    USART_Cmd(USART1, ENABLE);

    usartConfig.USART_BaudRate = 9600;
    usartConfig.USART_WordLength = USART_WordLength_8b;
    usartConfig.USART_StopBits = USART_StopBits_1;
    usartConfig.USART_Parity = USART_Parity_No;
    usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartConfig.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usartConfig);

    GPIO_InitTypeDef gpioConfig;

    //PA9 = USART1.TX => Alternative Function Output
    gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioConfig.GPIO_Pin = GPIO_Pin_9;
    gpioConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioConfig);

    //PA10 = USART1.RX => Input
    gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioConfig.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpioConfig);

    /* Enable RXNE interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* Enable USART1 global interrupt */
    NVIC_EnableIRQ(USART1_IRQn);
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
