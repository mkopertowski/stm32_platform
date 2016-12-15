#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#define OS_TIME_TO_TICKS(time_in_ms) pdMS_TO_TICKS(time_in_ms)
#define OS_TASK_NOTIFY_ALL_BITS 0xFFFFFFFF

#define APP_SOME_NOTIFICATION_NOTIF       (1 << 1)

struct context {
    TaskHandle_t app_task;
};

struct context ctx = { 0 };

#define OS_TASK_NOTIFY_FROM_ISR(task, value, action) \
        ({ \
                BaseType_t need_switch, ret; \
                ret = xTaskNotifyFromISR(task, value, action, &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })


static void some_action()
{
    OS_TASK_NOTIFY_FROM_ISR(ctx.app_task, APP_SOME_NOTIFICATION_NOTIF, eSetBits);
}

void app_task(void)
{
    trace_puts("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    for (;;) {
        BaseType_t ret;
        uint32_t notif;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TIME_TO_TICKS(50));

        if (ret == pdPASS) {
        }
    }
}
