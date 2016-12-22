#include <stdio.h>
#include <stdlib.h>
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

#define CMD_LENGTH_MAX (20)
#define CMD_RESPONSE_LENGTH_MAX (40)

#define QUEUE_MAX_ITEMS (5)
#define QUEUE_ITEM_SIZE (CMD_RESPONSE_LENGTH_MAX)

typedef struct {
    char cmdString[CMD_LENGTH_MAX];
} cmd_info_t;

typedef struct {
    uint8_t buffer[CMD_RESPONSE_LENGTH_MAX];
    uint8_t length;
} response_t;

struct context {
    TaskHandle_t task;
    QueueHandle_t queue;
    TimerHandle_t bt740_setup_timer;
    bt_cmd_t cmd;
    message_cb msg_cb;
};

static const cmd_info_t commands[] = {
        {""},                   // BT_CMD_UNKNOWN
        {"AT"},                 // BT_CMD_STATUS_CHECK
        {"ATE0"},               // BT_CMD_ECHO_OFF
        {"AT&W"},               // BT_CMD_WIRTE_S_REGISTER: write S register to non-volatile memory
};

static response_t cmdResponse;
static struct context ctx;

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
    xTaskCreate(bt740_task, "bt740_task", 1024, NULL, OS_TASK_PRIORITY, NULL);
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

static sendCmd(bt_cmd_t *cmd)
{
    // save command in the context
    memcpy(&ctx.cmd, cmd, sizeof(bt_cmd_t));

    // send command to BT740 module
    send_cmd_string(commands[cmd->type].cmdString);
}

void BT740_sendCmd(bt_cmd_t *cmd, bt_cmd_response_t *response)
{
    // save command in the context
    memcpy(&ctx.cmd, cmd, sizeof(bt_cmd_t));

    // send command to BT740 module
    send_cmd_string(commands[cmd->type].cmdString);
}

void BT740_register_for_messages(message_cb cb)
{
    ctx.msg_cb = cb;
}

void BT740_send_packet(uint8_t *data, uint8_t data_len)
{

}

static void queue_put_response() {
    uint8_t *item;

    item = malloc(CMD_RESPONSE_LENGTH_MAX);
    memcpy(item, cmdResponse.buffer, CMD_RESPONSE_LENGTH_MAX);

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
        if(cmdResponse.length == CMD_RESPONSE_LENGTH_MAX) {
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
                memset(cmdResponse.buffer, 0, CMD_RESPONSE_LENGTH_MAX);
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
    return (strncmp(commands[ctx.cmd.type].cmdString, response, strlen(commands[ctx.cmd.type].cmdString)) == 0);
}

void bt740_task(void *params)
{
    uint8_t *response;
    bt_cmd_t cmd;

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

                if(is_echoed_cmd(response)) {
                    free(response);
                    continue;
                }
                DEBUG_PRINTF("Received:%s\r\n",response);

                /* response handling here */

                free(response);
            }
            if (notification & BT740_SETUP_DONE_NOTIF) {
                cmd.type = BT_CMD_ECHO_OFF;
                sendCmd(&cmd);
            }
        }

        if(uxQueueMessagesWaiting(ctx.queue)) {
            OS_TASK_NOTIFY(ctx.task, BT740_RESPONSE_WAITING_NOTIF);
        }
    }
}
