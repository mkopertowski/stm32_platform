#include <stm32f10x_conf.h>

void Init_SPI1()
{
    // Initialization struct
    SPI_InitTypeDef SPI_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    // Step 1: Initialize SPI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    SPI_InitStruct.SPI_BaudRatePrescaler    = SPI_BaudRatePrescaler_128;
    SPI_InitStruct.SPI_CPHA                 = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_CPOL                 = SPI_CPOL_Low;
    SPI_InitStruct.SPI_DataSize             = SPI_DataSize_8b;
    SPI_InitStruct.SPI_Direction            = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_FirstBit             = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_Mode                 = SPI_Mode_Master;
    SPI_InitStruct.SPI_NSS                  = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);

    // Step 2: Initialize GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // GPIO pins for MOSI, MISO, and SCK
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

   //SPI ENABLE
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* Configure I/O for RES, D/C and CS */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

}

