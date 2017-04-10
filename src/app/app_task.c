#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>
#include <projdefs.h>
#include <os.h>
#include <storage.h>
#include <io.h>
#include <font.h>
#include <SSD1306.h>

//#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t app_task;
};

struct context ctx = { 0 };

static void button_state_listener(io_button_state_t state)
{
    switch(state) {
        case IO_BUTTON_SHORT_PRESS:
            OS_TASK_NOTIFY(ctx.app_task, APP_SHORT_PRESS_NOTIF);
            break;
        case IO_BUTTON_LONG_PRESS:
            OS_TASK_NOTIFY(ctx.app_task, APP_LONG_PRESS_NOTIF);
            break;
        default:
            DEBUG_PRINTF("APP: Unknown button state\r\n");
            break;
    }
}

static void module_hitted(void)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_MODULE_HITTED_NOTIF);
}

static void handle_module_hitted(void)
{
    DEBUG_PRINTF("APP: Module hitted\r\n");
}

void app_task(void *params)
{
    DEBUG_PRINTF("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    /* register button state listener */
    io_button_register_listener(button_state_listener);

    SSD1306_Draw_Text("1.23     1.05",10,1,Tahoma16,2);

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret != pdPASS) {
            continue;
        }

        if(notification & APP_SHORT_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Short button press\r\n");
        }

        if(notification & APP_LONG_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Long button press\r\n");
            /* ToDo: reset device (enable disoverable mode, unpair, reboot */
        }

        if(notification & APP_MODULE_HITTED_NOTIF) {
            handle_module_hitted();
        }
    }
}
