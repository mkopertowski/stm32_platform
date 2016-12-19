#include <global.h>

#if defined(DEBUG_ON)

#include <stdio.h>
#include <stdarg.h>
#include "diag/Trace.h"
#include "string.h"

#include "stm32f10x_usart.h"
#include "stm32f10x.h"

#define PRINTF_TMP_BUFF_SIZE (128)

void debug_init(void)
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
}

int debug(const char* format, ...)
{
    int ret;
    va_list ap;

    va_start (ap, format);

    static char buf[PRINTF_TMP_BUFF_SIZE];

    // Print to the local buffer
    ret = vsnprintf (buf, sizeof(buf), format, ap);
    if (ret > 0) {
        ret = _write(STDOUT_FILENO, buf, (size_t)ret);
    }

    va_end (ap);
    return ret;
}
#endif // DEBUG
