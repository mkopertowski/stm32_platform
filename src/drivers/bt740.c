#include <stdio.h>
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_usart.h"
#include "stm32f10x.h"

#include <global.h>
#include <os.h>
#include <bt740.h>

#define DEBUG_ON
#include <debug.h>

#define CMD_LENGTH_MAX (20)
#define RESPONSE_BUFFER_MAX (20)

typedef struct {
    char cmdString[CMD_LENGTH_MAX];
} cmd_info_t;

struct response {
    uint8_t buffer[RESPONSE_BUFFER_MAX];
    uint8_t index;
    bool ready;
};


struct context {
    TaskHandle_t task;
    bt_cmd_t cmd;
    message_cb msg_cb;
};

static const cmd_info_t commands[] = {
        {0},                   // BT_CMD_UNKNOWN
        {"AT"},                 // BT_CMD_STATUS_CHECK
        {"AT&W"},               // BT_CMD_WIRTE_S_REGISTER: write S register to non-volatile memory
};

static struct response cmdResponse;
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
    /* configure USART2 */
    usart_config();

    /* create task for communication with other devices */
    xTaskCreate(bt740_task, "bt740_task", 1024, NULL, OS_TASK_PRIORITY, NULL);
}

void send_char(char c)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, c);
    DEBUG_PRINTF("%c",c);
}

void send_string(const char *s)
{
    while (*s) {
        send_char(*s++);
    }
    DEBUG_PRINTF("\n");
}

void BT740_sendCmd(bt_cmd_t *cmd, bt_cmd_response_t *response)
{
    // save command in the context
    memcpy(&ctx.cmd, cmd, sizeof(bt_cmd_t));

    // send command to BT740 module
    send_string(commands[cmd->type].cmdString);
}

void BT740_register_for_messages(message_cb cb)
{
    ctx.msg_cb = cb;
}

void BT740_send_packet(uint8_t *data, uint8_t data_len)
{

}

void USART2_IRQHandler(void)
{
    uint8_t received_data;
    /* RXNE handler */

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        received_data = (uint8_t)USART_ReceiveData(USART2);
        cmdResponse.buffer[cmdResponse.index] = received_data;
        cmdResponse.index++;
        if(cmdResponse.index == RESPONSE_BUFFER_MAX) {
            cmdResponse.index = 0;
        }
        if(received_data == '\r') {
            bt_response_ready();
        }
    }
}

static void bt_response_ready(void)
{
    OS_TASK_NOTIFY(ctx.task, BT740_RESPONSE_READY_NOTIF);
}

void bt740_task(void *params)
{
    DEBUG_PRINTF("BT740 task started!\r\n");

    ctx.task = xTaskGetCurrentTaskHandle();

    send_string("AT\r");

    send_string("AT\r");
    send_string("AT\r");
    send_string("AT\r");
    send_string("AT\r");
    send_string("AT\r");
    send_string("AT\r");
    send_string("AT\r");


    for(;;) {
        BaseType_t ret;
        uint32_t notification;

        // Wait on any of the event group bits, then clear them all
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret == pdPASS) {
            if (notification & BT740_RESPONSE_READY_NOTIF) {
                DEBUG_PRINTF("Received:%s\r\n",cmdResponse.buffer);
            }
        }
    }
}
