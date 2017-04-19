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
#include <disp.h>
#include <measurements.h>

#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t task_handle;
    TimerHandle_t disp_update_timer;
};

static struct context ctx = { 0 };

static void button_state_listener(io_button_state_t state)
{
    switch(state) {
        case IO_BUTTON_SHORT_PRESS:
            OS_TASK_NOTIFY(ctx.task_handle, APP_SHORT_PRESS_NOTIF);
            break;
        case IO_BUTTON_LONG_PRESS:
            OS_TASK_NOTIFY(ctx.task_handle, APP_LONG_PRESS_NOTIF);
            break;
        default:
            DEBUG_PRINTF("APP: Unknown button state\r\n");
            break;
    }
}

static void handle_display_update_timer(TimerHandle_t xTimer)
{
    OS_TASK_NOTIFY(ctx.task_handle, MES_READ_O2CELLS_NOTIF);
}

void app_task(void *params)
{
    DEBUG_PRINTF("Application task started!\r\n");

    ctx.task_handle = xTaskGetCurrentTaskHandle();

    /* register button state listener */
    io_button_register_listener(button_state_listener);

    DSP_vInit();
    DSP_vShowDisplay(DSP_ID_STARTUP_QUESTION);

    ctx.disp_update_timer = xTimerCreate("app_tim",pdMS_TO_TICKS(1000),false,(void *)&ctx ,handle_display_update_timer);

    MES_vInit();

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
        }

    }
}
