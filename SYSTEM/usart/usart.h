#ifndef __USART_H
#define __USART_H	 
#include "sys.h"
#include "stdio.h"	

void uart_init(u32 bound); // USART1 ��ʼ��
void uart_send_status_height(int32_t height_cm);


#endif
