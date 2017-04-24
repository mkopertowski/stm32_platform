/*
 * ToDO:
 * - add storing bt address of received messages
 * - pairing
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
 * - SPP receiving messages
 */
#include <stdio.h>
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x.h"

#include <os.h>
#include <platform/ADS1115.h>
#include <platform/eeprom.h>
#include <platform/io.h>
#include <platform/spi.h>
#include <storage.h>

#define DEBUG_ON
#include <debug.h>

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

    /* Init SPI for LCD */
    Init_SPI1();

    /* A/D */
    ADS1115_init();

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
