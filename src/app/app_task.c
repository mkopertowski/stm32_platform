#include <stdio.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>
#include <projdefs.h>
#include <os.h>
#include <bt740.h>

#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t app_task;
    response_queue_t *response;
    bool response_status;
};

struct context ctx = { 0 };

void bt_module_state_cb(bt_state_t state)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_BT_MODULE_READY_NOTIF);
}

void bt_module_respone(bool status, response_queue_t *resp)
{
    ctx.response = resp;
    ctx.response_status = status;
    OS_TASK_NOTIFY(ctx.app_task, APP_BT_MODULE_RESPONSE_NOTIF);
}

static void handle_bt_module_response(void)
{
    response_queue_t *tmp_response;

    DEBUG_PRINTF("APP: Bluetooth module response arrived(%s)\r\n",(ctx.response_status ? "OK" : "ERROR"));

    while(ctx.response) {
        DEBUG_PRINTF("APP: Response: %s\r\n", ctx.response->data);
        tmp_response = ctx.response;
        ctx.response = ctx.response->next;
        free(tmp_response);
    }
}

void app_task(void *params)
{
    bt_cmd_t cmd;

    DEBUG_PRINTF("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    BT740_register_for_state(bt_module_state_cb);

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret != pdPASS) {
            continue;
        }

        if(notification & APP_BT_MODULE_READY_NOTIF) {
            DEBUG_PRINTF("APP: Bluetooth module is ready\r\n");
            cmd.type = BT_CMD_GET_DEVICES;
            BT740_sendCmd(&cmd, bt_module_respone);
        }

        if(notification & APP_BT_MODULE_RESPONSE_NOTIF) {
            handle_bt_module_response();
        }
    }
}
