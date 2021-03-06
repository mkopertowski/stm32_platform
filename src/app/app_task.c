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

//#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t app_task;
    response_queue_t *cmd_response;
    bt_status_t cmd_status;
    device_type_t device_type;
    bool send_message_status;
};

struct context ctx = { 0 };

static const bt_cmd_type_t init_commands_sequence[] = {
        BT_CMD_ECHO_OFF,
};

void bt_module_state_cb(bt_state_t state)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_BT_MODULE_READY_NOTIF);
}

static void bt_module_respone(bt_status_t status, response_queue_t *resp)
{
    ctx.cmd_response = resp;
    ctx.cmd_status = status;
    OS_TASK_NOTIFY(ctx.app_task, APP_BT_MODULE_RESPONSE_NOTIF);
}

static void handle_bt_module_response(void)
{
    response_queue_t *tmp_response;

    DEBUG_PRINTF("APP: Bluetooth module response arrived(%x)\r\n",ctx.cmd_status);

    while(ctx.cmd_response) {
        ctx.cmd_response->data[RESPONSE_DATA_LENGTH-1] = 0;
        DEBUG_PRINTF("APP: Response: %s\r\n", ctx.cmd_response->data);
        tmp_response = ctx.cmd_response;
        ctx.cmd_response = ctx.cmd_response->next;
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

static void module_hitted(void)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_MODULE_HITTED_NOTIF);
}

void spp_data_received(bool status, uint8_t *data, uint8_t data_len)
{
    ctx.send_message_status = status;
    OS_TASK_NOTIFY(ctx.app_task, APP_SPP_DATA_RECEIVED_NOTIF);
}

static void handle_module_hitted(void)
{
    DEBUG_PRINTF("APP: Module hitted\r\n");

}

void app_task(void *params)
{
    bt_cmd_t cmd;
    uint8_t bt_router_address[BT_ADDRESS_STR_LENGTH];

    DEBUG_PRINTF("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    /* register for BT module state */
    BT740_register_for_state(bt_module_state_cb);

    /* get device type from storage */
    storage_get_device_type(&ctx.device_type);
    DEBUG_PRINTF("Device type is %s, state:%s\r\n",((ctx.device_type == DEVICE_TYPE_AVALANCHE_BEACON)? "AVALANCHE_BEACON" : "AVALANCHE_ROUTER"),
                                                               (storage_is_paired() ? "PAIRED" : "NOT PAIRED"));

    /* register button state listener */
    io_button_register_listener(button_state_listener);

    /* register module hit listener */
    io_module_hit_register_listener(module_hitted);

    /* get router bluetooth address */
    memset(bt_router_address,0,BT_ADDRESS_STR_LENGTH);
    storage_get_router_bt_address(bt_router_address);

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

            /* get ready to receive messages */
            BT740_register_for_spp_data(spp_data_received);

            if(ctx.device_type == DEVICE_TYPE_AVALANCHE_BEACON) {
                uint8_t *data;
                data = malloc(4);
                memcpy(data,"DUPA",4);
                BT740_send_spp_data(bt_router_address,data,4);
            }
        }

        if(notification & APP_BT_MODULE_RESPONSE_NOTIF) {
            handle_bt_module_response();
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

        if(notification & APP_SPP_DATA_RECEIVED_NOTIF) {
            //DEBUG_PRINTF("APP: External message received(status=%d)\r\n",ctx.send_message_status);
            /* ToDo */
        }
    }
}
