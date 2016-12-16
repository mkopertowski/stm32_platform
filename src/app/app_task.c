#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>
#include <os.h>

#define APP_SOME_NOTIFICATION_NOTIF       (1 << 1)

struct context {
    TaskHandle_t app_task;
};

struct context ctx = { 0 };

static void some_action()
{
    OS_TASK_NOTIFY(ctx.app_task, APP_SOME_NOTIFICATION_NOTIF);
}

void app_task(void)
{
    trace_puts("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    for (;;) {
        BaseType_t ret;
        uint32_t notif;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notif, pdMS_TO_TICKS(50));

        if (ret == pdPASS) {
        }
    }
}
