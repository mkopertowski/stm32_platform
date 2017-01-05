/*
 * ToDO:
 * - pairing
 * - SPP receiving messages
 * - self and router address storing
 * - long press reset
 * - sending comands to router (I'm online, Module was hitted)
 * - router commands (hello, enable avalanche beacon)
 *
 * Done:
 * - module storage
 * - thread safe BT740_sendCmd() + callback
 * - device type to storage (router, end device)
 * - led API
 * - button handling
 * - get friendly name (command)
 * - SPP sending messages protocol
 *
 */
#include <stdio.h>
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x.h"

#include <bt740.h>
#include <eeprom.h>
#include <os.h>
#include <storage.h>
#include <io.h>

//#define DEBUG_ON
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
    DEBUG_INIT();
    DEBUG_PRINTF("System clock: %u Hz\r\n", SystemCoreClock);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* IO */
    io_init();

    /* init flash data storage */
    storage_init();

    /* init BT740 bluetooth module */
    BT740_init();

    xTaskCreate(app_task, "app_task", 512, NULL, OS_TASK_PRIORITY, NULL);

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
