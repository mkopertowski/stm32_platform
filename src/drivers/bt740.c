#include <drivers/bt740.h>
#include "stm32f10x_usart.h"
#include "stm32f10x.h"

#define CMD_LENGTH_MAX (20)

typedef struct {
    char cmdString[CMD_LENGTH_MAX];
} CMD_INFO;



void BT740_init(void)
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


/**********************************************************
 * USART1 interrupt request handler: on reception of a
 * character 't', toggle LED and transmit a character 'T'
 *********************************************************/
void USART1_IRQHandler(void)
{
    /* RXNE handler */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* If received 't', toggle LED and transmit 'T' */
        if((char)USART_ReceiveData(USART1) == 't')
        {
            USART_SendData(USART1, 'T');
        }
    }

    /* ------------------------------------------------------------ */
    /* Other USART1 interrupts handler can go here ...             */
}

