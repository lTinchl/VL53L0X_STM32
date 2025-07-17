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

// ֧�� printf �������������
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0)
        ; // �ȴ����ͻ�������
    USART1->DR = (u8)ch;
    return ch;
}

// USART1 ���͹��ܳ�ʼ������ TX��
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

    // RX -> PA10���Ǳ��룩
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 ��������
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx; // ֻ���� TX

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE); // ���� USART1
}

void uart_send_status_height(int32_t height_cm)
{
    uint8_t buf[17];
    uint8_t sum = 0;
    uint8_t i = 0;
    int16_t dummy = 0; // �Ƕ�ռλ

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0x01; // ������
    buf[3] = 12;   // ���ݳ���

    // Roll
    buf[4] = (dummy >> 8) & 0xFF;
    buf[5] = dummy & 0xFF;
    // Pitch
    buf[6] = (dummy >> 8) & 0xFF;
    buf[7] = dummy & 0xFF;
    // Yaw
    buf[8] = (dummy >> 8) & 0xFF;
    buf[9] = dummy & 0xFF;

    // �߶�4�ֽ�
    buf[10] = (height_cm >> 24) & 0xFF;
    buf[11] = (height_cm >> 16) & 0xFF;
    buf[12] = (height_cm >> 8) & 0xFF;
    buf[13] = height_cm & 0xFF;

    buf[14] = 0x01;              // ����ģʽ
    buf[15] = 0x00; // ����״̬

    // У���
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
