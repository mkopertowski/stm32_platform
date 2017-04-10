#include <stm32f10x_conf.h>
#include <stm32f10x_i2c.h>
#include <ADS1115.h>

#define ADS1115_ADDR 0x0A

void ADS1115_init()
{
    I2C_InitTypeDef i2c;
    GPIO_InitTypeDef gpio;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; // SCL, SDA
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);


    I2C_StructInit(&i2c);
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);
}

static void I2C_Start(void)
{
    I2C_GenerateSTART(I2C1, ENABLE);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
}

static void I2C_Stop(void)
{
    I2C_GenerateSTOP(I2C1, ENABLE);
}

static void I2C_SetupWrite(void)
{
    I2C_Send7bitAddress(I2C1, ADS1115_ADDR, I2C_Direction_Transmitter);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
}

static void I2C_SetupRead(void)
{
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Send7bitAddress(I2C1, ADS1115_ADDR, I2C_Direction_Receiver);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
}

static void I2C_Write(uint8_t data)
{
    I2C_SendData(I2C1, data);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
}

static uint8_t I2C_GetByte(void)
{
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
    return I2C_ReceiveData(I2C1);
}

void ADS1115_write(unsigned char pointer, unsigned int value)
{
    I2C_Start();
    I2C_SetupWrite();
    I2C_Write(pointer);
    I2C_Write((value & 0xFF00) >> 8);
    I2C_Write(value & 0x00FF);
    I2C_Stop();
}

void ADS1115_configure(unsigned int value)
{
     ADS1115_write(ADS1115_config_reg_pointer, value);
}

void ADS1115_write_thresholds(unsigned char reg_select, unsigned int value)
{
     unsigned char ptr = 0x00;
     
     switch(reg_select)
     {
         case high_reg:
         {
              ptr = ADS1115_Hi_Thres_reg_pointer;
              break;
         }
         case low_reg:
         {
              ptr = ADS1115_Lo_Thres_reg_pointer;
              break;
         }
     }
     
     ADS1115_write(ptr, value);
}


unsigned int ADS1115_read(unsigned char pointer)
{
     unsigned char lb = 0x00;
     unsigned int hb = 0x0000;
     
     I2C_Start();
     I2C_SetupWrite();
     I2C_Write(pointer);
     I2C_Stop();
     
     I2C_Start();
     I2C_SetupRead();
     hb = I2C_GetByte();

     /* nack for the last byte */
     I2C_AcknowledgeConfig(I2C1, DISABLE);
     I2C_Stop();
     /* read last byte */
     lb = I2C_GetByte();
     
     hb <<= 8;
     hb |= lb;
     
     return hb;
}
