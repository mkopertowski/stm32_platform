//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x.h"

#include <bt740.h>
#include <os.h>

#define DEBUG_ON
#include <debug.h>

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void app_task(void *params);

int main(int argc, char* argv[])
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    DEBUG_INIT();
    DEBUG_PRINTF("System clock: %u Hz\r\n", SystemCoreClock);

    BT740_init();

    //xTaskCreate(app_task, "app_task", 512, NULL, OS_TASK_PRIORITY, NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();

    // Infinite loop
    for(;;);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    for(;;);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
