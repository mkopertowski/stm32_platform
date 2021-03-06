/*
e * ORANGE GND
 * BROWN  TX    A3(RX)
 * YELLOW RX    A2(TX)
 * BLUE   VCC
 *
 * FT232 blue gnd
 *       green rxd
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "stm32f10x_usart.h"
#include "stm32f10x.h"

#include <global.h>
#include <os.h>
#include <bt740.h>

#define DEBUG_ON
#include <debug.h>

#define CMD_LENGTH_MAX (30)

#define QUEUE_MAX_ITEMS (5)
#define QUEUE_ITEM_SIZE (RESPONSE_DATA_LENGTH)

typedef struct {
    char cmdString[CMD_LENGTH_MAX];
} cmd_info_t;

typedef struct {
    uint8_t buffer[RESPONSE_DATA_LENGTH];
    uint8_t length;
} response_t;

typedef struct {
    uint8_t bt_address[BT_ADDRESS_STR_LENGTH];
    uint8_t *data;
    uint8_t data_len;
    TimerHandle_t escape_timer;
    bool connection_active;
    data_receive_cb receive_cb;
    bt_status_t status;
} spp_t;

struct context {
    TaskHandle_t task;
    QueueHandle_t queue;
    TimerHandle_t bt740_setup_timer;
    bt_cmd_t cmd;
    bool processing_cmd;
    uint8_t cmdString[CMD_LENGTH_MAX];
    cmd_response_cb cmd_response_cb;
    response_queue_t *response_queue;
    response_queue_t *response_queue_tail;
    state_cb module_state_cb;
    spp_t spp;
};

static const cmd_info_t commands[] = {
        {"AT"},                 // BT_CMD_STATUS_CHECK
        {"ATE0"},               // BT_CMD_ECHO_OFF
        {"AT+BTT?"},            // BT_CMD_GET_DEVICES
        {"AT&W"},               // BT_CMD_WIRTE_S_REGISTER: write S register to non-volatile memory
        {"AT+BTF%s"},           // BT_CMD_GET_FRIENDLY_NAME
        {"ATD%s,1101"},         // BT_CMD_SPP_START
        {"ATH"}                 // BT_CMD_SPP_STOP
};

static response_t cmdResponse;
static struct context ctx = {0};

static void bt740_task(void *params);
static void bt_response_ready(void);

static void usart_config(void)
{
    USART_InitTypeDef usartConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_Cmd(USART2, ENABLE);

    usartConfig.USART_BaudRate = 9600;
    usartConfig.USART_WordLength = USART_WordLength_8b;
    usartConfig.USART_StopBits = USART_StopBits_1;
    usartConfig.USART_Parity = USART_Parity_No;
    usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartConfig.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &usartConfig);

    GPIO_InitTypeDef gpioConfig;

    //PA2 = USART2.TX => Alternative Function Output
    gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioConfig.GPIO_Pin = GPIO_Pin_2;
    gpioConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioConfig);

    //PA3 = USART2.RX => Input
    gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioConfig.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &gpioConfig);

    /* Enable RXNE interrupt */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    /* Enable USART2 global interrupt */
    NVIC_EnableIRQ(USART2_IRQn);
}

void BT740_init(void)
{
    /* create task for communication with other devices */
    xTaskCreate(bt740_task, "bt740_task", 3072, NULL, OS_TASK_PRIORITY, NULL);
}

void send_char(char c)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, c);
    DEBUG_PRINTF("%c",c);
}

void send_buffer(uint8_t *data, uint8_t data_len)
{
    while(data_len) {
        send_char(*data++);
        data_len--;
    }
}

void send_cmd_string(const char *s)
{
    while (*s) {
        send_char(*s++);
    }
    send_char('\r'); // send \r character
    DEBUG_PRINTF("\n");
}

void BT740_sendCmd(bt_cmd_t *cmd, cmd_response_cb cb)
{
    // save command in the context
    memcpy(&ctx.cmd, cmd, sizeof(bt_cmd_t));
    ctx.cmd_response_cb = cb;

    OS_TASK_NOTIFY(ctx.task, BT740_SEND_COMMAND_NOTIF);
}

void BT740_register_for_spp_data(data_receive_cb cb)
{
    ctx.spp.receive_cb = cb;
}

void BT740_response_free(response_queue_t *resp)
{
    response_queue_t *p;

    while(resp) {
        p = resp;
        resp = resp->next;
        free(p);
    }
}

static void bt_spp_start_respone(bt_status_t status, response_queue_t *resp)
{
    switch(status) {
        case BT_STATUS_CONNECTED:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_CONNECTED_NOTIF);
            break;
        case BT_STATUS_ERROR:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_ERROR_NOTIF);
            break;
        case BT_STATUS_NO_CARRIER:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_NO_CARRIER_NOTIF);
            break;
        default:
            break;
    }
    BT740_response_free(resp);
}

static void bt_spp_stop_respone(bt_status_t status, response_queue_t *resp)
{
    ctx.spp.status = status;
    OS_TASK_NOTIFY(ctx.task, BT740_SPP_DISCONNECT_NOTIF);
    BT740_response_free(resp);
}

void BT740_send_spp_data(uint8_t bt_address[], uint8_t *data, uint8_t data_len)
{
    bt_cmd_t cmd;

    if(ctx.spp.connection_active) {
        return;
    } else {
        ctx.spp.connection_active = true;
    }

    ctx.spp.data_len = data_len;
    ctx.spp.data = data;
    memcpy(ctx.spp.bt_address,bt_address,BT_ADDRESS_STR_LENGTH);

    cmd.type = BT_CMD_SPP_START;
    sprintf(cmd.params.bt_address,"%s",bt_address);

    BT740_sendCmd(&cmd,bt_spp_start_respone);

    return;
}

static void queue_put_response() {
    uint8_t *item;

    item = malloc(RESPONSE_DATA_LENGTH);
    if(!item) {
        return;
    }

    memcpy(item, cmdResponse.buffer, RESPONSE_DATA_LENGTH);
    OS_QUEUE_PUT_FROM_ISR(ctx.queue, &item);
}

void USART2_IRQHandler(void)
{
    uint8_t received_data;
    /* RXNE handler */

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        received_data = (uint8_t)USART_ReceiveData(USART2);
        cmdResponse.buffer[cmdResponse.length] = received_data;
        cmdResponse.length++;
        if(cmdResponse.length == RESPONSE_DATA_LENGTH) {
            cmdResponse.length = 0;
        }
        if(received_data == '\r') {
            if(cmdResponse.length == 1) {
                cmdResponse.length = 0;
                cmdResponse.buffer[0] = 0;
            } else {
                queue_put_response();
                bt_response_ready();
                cmdResponse.length = 0;
                memset(cmdResponse.buffer, 0, RESPONSE_DATA_LENGTH);
            }
        } else if((received_data == '\n') && (cmdResponse.length == 1)) {
            cmdResponse.length = 0;
            cmdResponse.buffer[0] = 0;
        } else {
            /* wait for some more bytes */
        }
    }
}

static void bt_response_ready(void)
{
    OS_TASK_NOTIFY(ctx.task, BT740_RESPONSE_WAITING_NOTIF);
}

static void bt740_ready(TimerHandle_t xTimer)
{
    OS_TASK_NOTIFY(ctx.task, BT740_SETUP_DONE_NOTIF);
}

static bool is_echoed_cmd(uint8_t *response)
{
    return (strncmp(ctx.cmdString, (char*)response, strlen(ctx.cmdString)) == 0);
}

static void handleCmd(void)
{
    memset(ctx.cmdString,0,CMD_LENGTH_MAX);
    // send command to BT740 module
    switch(ctx.cmd.type) {
        case BT_CMD_GET_FRIENDLY_NAME:
        case BT_CMD_SPP_START:
            sprintf(ctx.cmdString,commands[ctx.cmd.type].cmdString,ctx.cmd.params.bt_address);
            break;
        default:
            sprintf(ctx.cmdString,"%s",commands[ctx.cmd.type].cmdString);
            break;
    }
    send_cmd_string(ctx.cmdString);
    ctx.processing_cmd = true;
}

void BT740_register_for_state(state_cb cb)
{
    ctx.module_state_cb = cb;
}

static spp_stop(void)
{
    bt_cmd_t cmd;
    cmd.type = BT_CMD_SPP_STOP;
    BT740_sendCmd(&cmd,bt_spp_stop_respone);
}

static void handleResponse(uint8_t *response)
{
    if(ctx.processing_cmd && is_echoed_cmd(response)) {
        free(response);
        return;
    }

    DEBUG_PRINTF("Received:%s\r\n",response);

    if(strncmp("OK", (char*)response, strlen("OK")) == 0) {
        if(ctx.spp.connection_active) {
            spp_stop();
        } else if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_STATUS_OK, ctx.response_queue);
            ctx.response_queue = NULL;
            ctx.cmd_response_cb = NULL;
        }
        ctx.processing_cmd = false;
    } else if(strncmp("ERROR", (char*)response, strlen("ERROR")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_STATUS_ERROR, ctx.response_queue);
            ctx.response_queue = NULL;
            ctx.cmd_response_cb = NULL;
        }
        ctx.processing_cmd = false;
    } else if(strncmp("CONNECT", (char*)response, strlen("CONNECT")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_STATUS_CONNECTED, NULL);
            ctx.response_queue = NULL;
            ctx.cmd_response_cb = NULL;
        } else if (ctx.spp.connection_active) {
            if(ctx.spp.receive_cb) {
                ctx.spp.receive_cb(BT_STATUS_CONNECTED,NULL,0);
            }
        }
        ctx.processing_cmd = false;
    } else if(strncmp("NO CARRIER", (char*)response, strlen("NO CARRIER")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_STATUS_NO_CARRIER, NULL);
            ctx.response_queue = NULL;
            ctx.cmd_response_cb = NULL;
        } else if (ctx.spp.connection_active) {
            if(ctx.spp.receive_cb) {
                ctx.spp.receive_cb(BT_STATUS_NO_CARRIER,NULL,0);
            }
        }
        ctx.processing_cmd = false;
    } else if(strncmp("RING", (char*)response, strlen("RING")) == 0) {
        ctx.spp.connection_active = true;
        if(ctx.spp.receive_cb) {
            ctx.spp.receive_cb(BT_STATUS_RING,NULL,0);
        }
    } else {
        response_queue_t *item;

        if(ctx.spp.connection_active) {
            uint8_t len = strlen(response);

            ctx.spp.data = malloc(len+1);
            if(!ctx.spp.data) {
                return;
            }
            sprintf(ctx.spp.data,"%s",response);
            ctx.spp.data_len = len;
            ctx.spp.receive_cb(BT_STATUS_DATA,ctx.spp.data,ctx.spp.data_len);
        } else {
            item = (response_queue_t*)malloc(sizeof(response_queue_t));
            if(!item) {
                return;
            }

            if(BT_STATUS_NO_CARRIER)

            memcpy(item->data, response, RESPONSE_DATA_LENGTH);
            item->next = NULL;

            if(!ctx.response_queue) {
                ctx.response_queue = item;
                ctx.response_queue_tail = item;
            } else {
                ctx.response_queue_tail->next = item;
                ctx.response_queue_tail = item;
            }
        }
    }

    /* response handling here */
    free(response);
}

static volatile uint8_t spp_escape_count;

static void send_spp_escape_sequence(void)
{
    spp_escape_count = 3;
    xTimerStart(ctx.spp.escape_timer, 0 );
}

static void spp_escape(TimerHandle_t xTimer)
{
    if(spp_escape_count == 0) {
        return;
    }
    OS_TASK_NOTIFY(ctx.task, BT740_SPP_SEND_ESCAPE_CHAR_NOTIF);
    spp_escape_count--;
    xTimerStart(ctx.spp.escape_timer, 0 );
}

void handle_spp_connection(uint32_t notification)
{
    if (notification & BT740_SPP_CONNECTED_NOTIF) {
        /* send data */
        send_buffer(ctx.spp.data,ctx.spp.data_len);
        /* send escape sequence */
        send_spp_escape_sequence();
        /* ToDo: start response timer */
    }

    if ((notification & BT740_SPP_ERROR_NOTIF) ||
        (notification & BT740_SPP_NO_CARRIER_NOTIF)) {
        if(ctx.spp.receive_cb) {
            ctx.spp.receive_cb(false,NULL,0);
        }
        ctx.spp.connection_active = false;
    }

    if (notification & BT740_SPP_DISCONNECT_NOTIF) {
        if(ctx.spp.receive_cb) {
            bool status = false;

            if(ctx.spp.status == BT_STATUS_OK) {
                status = true;
            }
            ctx.spp.receive_cb(status,NULL,0);
        }
        ctx.spp.connection_active = false;
    }

    if(notification & BT740_SPP_SEND_ESCAPE_CHAR_NOTIF) {
        send_buffer("^",1);
    }
}

void bt740_task(void *params)
{
    uint8_t *response;

    DEBUG_PRINTF("BT740 task started!\r\n");

    ctx.task = xTaskGetCurrentTaskHandle();
    ctx.queue = xQueueCreate(QUEUE_MAX_ITEMS, QUEUE_ITEM_SIZE);

    ctx.bt740_setup_timer = xTimerCreate("bt740_tim",pdMS_TO_TICKS(1500),false,(void *)&ctx ,bt740_ready);
    ctx.spp.escape_timer = xTimerCreate("spp_tim",pdMS_TO_TICKS(200),false,(void *)&ctx ,spp_escape);

    /* configure USART2 */
    usart_config();

    xTimerStart(ctx.bt740_setup_timer, 0 );

    for(;;) {
        BaseType_t ret;
        uint32_t notification;

        // Wait on any of the event group bits, then clear them all
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret == pdPASS) {
            if (notification & BT740_RESPONSE_WAITING_NOTIF) {
                if(!xQueueReceive(ctx.queue, &response, 0)) {
                    continue;
                }

                handleResponse(response);
            }
            if (notification & BT740_SETUP_DONE_NOTIF) {
                if(ctx.module_state_cb) {
                    ctx.module_state_cb(BT_MODULE_READY);
                }
            }

            if (notification & BT740_SEND_COMMAND_NOTIF) {
                handleCmd();
            }

            handle_spp_connection(notification);
        }

        if(uxQueueMessagesWaiting(ctx.queue)) {
            OS_TASK_NOTIFY(ctx.task, BT740_RESPONSE_WAITING_NOTIF);
        }
    }
}
