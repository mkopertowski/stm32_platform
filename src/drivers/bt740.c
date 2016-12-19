#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_usart.h"
#include "stm32f10x.h"

#include <global.h>
#include <os.h>
#include <bt740.h>

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

static void usart_config(void)
{
    USART_InitTypeDef usartConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    USART_Cmd(USART1, ENABLE);

    usartConfig.USART_BaudRate = 9600;
    usartConfig.USART_WordLength = USART_WordLength_8b;
    usartConfig.USART_StopBits = USART_StopBits_1;
    usartConfig.USART_Parity = USART_Parity_No;
    usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartConfig.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usartConfig);

    GPIO_InitTypeDef gpioConfig;

    //PA9 = USART1.TX => Alternative Function Output
    gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioConfig.GPIO_Pin = GPIO_Pin_9;
    gpioConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioConfig);

    //PA10 = USART1.RX => Input
    gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioConfig.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpioConfig);

    /* Enable RXNE interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* Enable USART1 global interrupt */
    NVIC_EnableIRQ(USART1_IRQn);
}

void BT740_init(void)
{
    /* configure usart */
    usart_config();

    /* create task for communication with other devices */
    xTaskCreate(bt740_task, "bt740_task", configMINIMAL_STACK_SIZE, NULL, OS_TASK_PRIORITY, NULL);
}

void send_char(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

void send_string(const char *s)
{
    while (*s)
        send_char(*s++);
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

void USART1_IRQHandler(void)
{
    /* RXNE handler */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        cmdResponse.buffer[cmdResponse.index] = (uint8_t)USART_ReceiveData(USART1);
        cmdResponse.index++;
        if(cmdResponse.index == RESPONSE_BUFFER_MAX) {
            cmdResponse.index = 0;
        }
    }
}

void bt_recevie(void)
{
    OS_TASK_NOTIFY(ctx.task, BT740_RECEIVE_NOTIF);
}

void bt740_task(void *params)
{
    trace_puts("BT740 task started!\r\n");

    ctx.task = xTaskGetCurrentTaskHandle();

    for(;;) {
        BaseType_t ret;
        uint32_t notification;

        // Wait on any of the event group bits, then clear them all
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret == pdPASS) {
            if (notification & BT740_RECEIVE_NOTIF) {
                trace_puts("Task notified by some action!\r\n");
            }
        }
    }
}
