#include "usart.h"
static uint8_t local_lock = 0;

#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};

FILE __stdout;

_sys_exit(int x)
{
    x = x;
}

// 支持 printf 函数输出到串口
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0)
        ; // 等待发送缓冲区空
    USART1->DR = (u8)ch;
    return ch;
}

// USART1 发送功能初始化（仅 TX）
void uart_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // TX -> PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX -> PA10（非必须）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 参数配置
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx; // 只启用 TX

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE); // 启用 USART1
}

void uart_send_status_height(int32_t height_cm)
{
    uint8_t buf[17];
    uint8_t sum = 0;
    uint8_t i = 0;
    int16_t dummy = 0; // 角度占位

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0x01; // 功能字
    buf[3] = 12;   // 数据长度

    // Roll
    buf[4] = (dummy >> 8) & 0xFF;
    buf[5] = dummy & 0xFF;
    // Pitch
    buf[6] = (dummy >> 8) & 0xFF;
    buf[7] = dummy & 0xFF;
    // Yaw
    buf[8] = (dummy >> 8) & 0xFF;
    buf[9] = dummy & 0xFF;

    // 高度4字节
    buf[10] = (height_cm >> 24) & 0xFF;
    buf[11] = (height_cm >> 16) & 0xFF;
    buf[12] = (height_cm >> 8) & 0xFF;
    buf[13] = height_cm & 0xFF;

    buf[14] = 0x01;              // 飞行模式
    buf[15] = 0x00; // 锁定状态

    // 校验和
    for (i = 0; i < 16; i++)
        sum += buf[i];
    buf[16] = sum;

    for (i = 0; i < 17; i++)
    {
        USART_SendData(USART1, buf[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    }
}
