//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

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

int main(int argc, char* argv[])
{
    BT740_init();

    // Send a greeting to the trace device (skipped on Release).
    trace_puts("Hello ARM World!");

    // At this stage the system clock should have already been configured
    // at high speed.
    trace_printf("System clock: %u Hz\n", SystemCoreClock);

    timer_start();
  
    xTaskCreate( app_task, "app_task", configMINIMAL_STACK_SIZE, NULL, OS_TASK_PRIORITY, NULL );

    /* Start the scheduler. */
    vTaskStartScheduler();

    // Infinite loop
    for(;;);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
