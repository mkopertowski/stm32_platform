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
#include <bt740.h>
#include <storage.h>
#include <io.h>

#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t app_task;
    response_queue_t *response;
    bool response_status;
    device_type_t device_type;
};

struct context ctx = { 0 };

static const bt_cmd_type_t init_commands_sequence[] = {
        BT_CMD_ECHO_OFF,
};

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
        ctx.response->data[RESPONSE_DATA_LENGTH-1] = 0;
        DEBUG_PRINTF("APP: Response: %s\r\n", ctx.response->data);
        tmp_response = ctx.response;
        ctx.response = ctx.response->next;
        free(tmp_response);
    }
}

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

void app_task(void *params)
{
    bt_cmd_t cmd;

    DEBUG_PRINTF("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    /* register for BT module state */
    BT740_register_for_state(bt_module_state_cb);

    /* get device type from storage */
    storage_get_device_type(&ctx.device_type);
    DEBUG_PRINTF("Device type is %s, state:%s\r\n",(ctx.device_type ? "AVALANCHE_BEACON" : "AVALANCHE_ROUTER"),
                                                               (storage_is_paired() ? "PAIRED" : "NOT PAIRED"));

    /* register button state listener */
    io_button_register_listener(button_state_listener);

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
            cmd.type = BT_CMD_GET_FRIENDLY_NAME;
            memcpy(cmd.params.bt_address,"4040A7BE6AC8",BT_ADDRESS_LENGTH);
            BT740_sendCmd(&cmd, bt_module_respone);
        }

        if(notification & APP_BT_MODULE_RESPONSE_NOTIF) {
            handle_bt_module_response();
        }

        if(notification & APP_SHORT_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Short button press\r\n");
        }

        if(notification & APP_LONG_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Long button press\r\n");
        }

    }
}
