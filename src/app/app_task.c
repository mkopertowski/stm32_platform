#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>
#include <projdefs.h>
#include <os.h>

struct context {
    TaskHandle_t app_task;
};

struct context ctx = { 0 };

void app_some_action(void)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_SOME_NOTIFICATION_NOTIF);
}

void app_task(void *params)
{
    uint8_t send_notif = 0;

    trace_printf("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if (ret == pdPASS) {
            if (notification & APP_SOME_NOTIFICATION_NOTIF) {
                trace_printf("Task notified by some action!\r\n");
            }
        } else if (!send_notif) {
            app_some_action();
            send_notif = 1;
        }
    }
}
