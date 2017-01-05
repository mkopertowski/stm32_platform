/*
 * ORANGE GND
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

struct context {
    TaskHandle_t task;
    QueueHandle_t queue;
    TimerHandle_t bt740_setup_timer;
    bt_cmd_t cmd;
    cmd_response_cb cmd_response_cb;
    response_queue_t *response_queue;
    response_queue_t *response_queue_tail;
    msg_receive_cb receive_cb;
    state_cb module_state_cb;
    bool spp_connection_active;
    bt_packet_t packet;
};

static const cmd_info_t commands[] = {
        {""},                   // BT_CMD_UNKNOWN
        {"AT"},                 // BT_CMD_STATUS_CHECK
        {"ATE0"},               // BT_CMD_ECHO_OFF
        {"AT+BTT?"},            // BT_CMD_GET_DEVICES
        {"AT&W"},               // BT_CMD_WIRTE_S_REGISTER: write S register to non-volatile memory
        {"AT+BTF%s"},           // BT_CMD_GET_FRIENDLY_NAME
        {"ATD%s,1101"},         // BT_CMD_SPP_START
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
    xTaskCreate(bt740_task, "bt740_task", 2048, NULL, OS_TASK_PRIORITY, NULL);
}

void send_char(char c)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, c);
    DEBUG_PRINTF("%c",c);
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

void BT740_register_for_messages(msg_receive_cb cb)
{
    ctx.receive_cb = cb;
}

static void bt_module_respone(bt_cmd_status_t status, response_queue_t *resp)
{
    switch(status) {
        case BT_CMD_STATUS_CONNECTED:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_CONNECT_NOTIF);
            break;
        case BT_CMD_STATUS_ERROR:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_ERROR_NOTIF);
            break;
        case BT_CMD_STATUS_NO_CARRIER:
            OS_TASK_NOTIFY(ctx.task, BT740_SPP_NO_CARRIER_NOTIF);
            break;
        default:
            break;
    }
}

void BT740_send_message(bt_packet_t *packet)
{
    bt_cmd_t cmd;

    if(ctx.spp_connection_active) {
        return;
    } else {
        ctx.spp_connection_active = true;
    }

    memcpy(&ctx.packet,packet,sizeof(bt_packet_t));

    cmd.type = BT_CMD_SPP_START;
    sprintf(cmd.params.bt_address,"%s",packet->bt_address);

    BT740_sendCmd(&cmd,bt_module_respone);

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
    return (strncmp(commands[ctx.cmd.type].cmdString, (char*)response, strlen(commands[ctx.cmd.type].cmdString)) == 0);
}

static void handleCmd(void)
{
    uint8_t cmd[CMD_LENGTH_MAX];

    memset(cmd,0,CMD_LENGTH_MAX);
    // send command to BT740 module
    switch(ctx.cmd.type) {
        case BT_CMD_GET_FRIENDLY_NAME:
            sprintf(cmd,commands[ctx.cmd.type].cmdString,ctx.cmd.params.bt_address);
            send_cmd_string(cmd);
            break;
        default:
            send_cmd_string(commands[ctx.cmd.type].cmdString);
            break;
    }
}

void BT740_register_for_state(state_cb cb)
{
    ctx.module_state_cb = cb;
}

static void handleResponse(uint8_t *response)
{
    if(is_echoed_cmd(response)) {
        free(response);
        return;
    }

    DEBUG_PRINTF("Received:%s\r\n",response);

    if(strncmp("OK", (char*)response, strlen("OK")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_CMD_STATUS_OK, ctx.response_queue);
            ctx.response_queue = NULL;
        }
    } else if(strncmp("ERROR", (char*)response, strlen("ERROR")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_CMD_STATUS_ERROR, ctx.response_queue);
            ctx.response_queue = NULL;
        }
    } else if(strncmp("CONNECT", (char*)response, strlen("CONNECT")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_CMD_STATUS_CONNECTED, NULL);
            ctx.response_queue = NULL;
        }
    } else if(strncmp("NO CARRIER", (char*)response, strlen("NO CARRIER")) == 0) {
        if(ctx.cmd_response_cb) {
            ctx.cmd_response_cb(BT_CMD_STATUS_NO_CARRIER, NULL);
            ctx.response_queue = NULL;
        }
    } else {
        response_queue_t *item;

        item = (response_queue_t*)malloc(sizeof(response_queue_t));
        if(!item) {
            return;
        }

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

    /* response handling here */
    free(response);
}

void bt740_task(void *params)
{
    uint8_t *response;

    DEBUG_PRINTF("BT740 task started!\r\n");

    ctx.task = xTaskGetCurrentTaskHandle();
    ctx.queue = xQueueCreate(QUEUE_MAX_ITEMS, QUEUE_ITEM_SIZE);

    ctx.bt740_setup_timer = xTimerCreate("bt740_tim",pdMS_TO_TICKS(1500),false,(void *)&ctx ,bt740_ready);

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

            if (notification & BT740_SPP_CONNECT_NOTIF) {
                /* send packet */

                /* send escape sequence */

                /* disconnect */

                /* ToDo: start response timer */
            }

            if (notification & BT740_SPP_NO_CARRIER_NOTIF) {
                if(ctx.receive_cb) {
                    ctx.receive_cb(false,NULL);
                }
                ctx.spp_connection_active = false;
            }

            if (notification & BT740_SPP_ERROR_NOTIF) {
                if(ctx.receive_cb) {
                    ctx.receive_cb(false,NULL);
                }
                ctx.spp_connection_active = false;
            }
        }

        if(uxQueueMessagesWaiting(ctx.queue)) {
            OS_TASK_NOTIFY(ctx.task, BT740_RESPONSE_WAITING_NOTIF);
        }
    }
}
