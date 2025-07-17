#include "stm32f10x.h" //stm32头文件
#include "usart.h"
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "vl53l0x.h"
#include "vl53l0x_gen.h"
#include "OLED.h"

extern void uart_send_status_height(int32_t height_cm);
/******************************************************************************/
extern VL53L0X_Dev_t vl53l0x_dev; // 设备I2C数据参数

GPIO_InitTypeDef GPIO_InitStructure;
/******************************************************************************/
int main(void)
{

	uint8_t data;
	uint8_t len;
	uint16_t word;
	uint8_t data1;
	int32_t height_cm = 0;

	
	static char buf[VL53L0X_MAX_STRING_LENGTH]; // 测试模式字符串字符缓冲区
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;	// 工作状态
	u8 mode = 1;								// 0：默认；1：高精度；2：长距离；3：高速度
	OLED_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 分组2
	uart_init(115200);								// 串口1
	delay_init();
	delay_ms(200);

	if (vl53l0x_init(&vl53l0x_dev)) // vl53l0x初始化
	{
		printf("VL53L0X_Init Error!!!\r\n");
		delay_ms(200);
	}
	else
	{
		printf("VL53L0X_Init OK\r\n");

		VL53L0X_RdByte(&vl53l0x_dev, 0xC0, &data);
		printf("regc0 = 0x%x\n\r", data);

		VL53L0X_RdByte(&vl53l0x_dev, 0xC1, &data1);
		printf("regc1 = 0x%x\n\r", data1);

		VL53L0X_RdByte(&vl53l0x_dev, 0xC2, &len);
		printf("regc2 = 0x%x\n\r", len);

		Status = VL53L0X_WrByte(&vl53l0x_dev, 0x51, 0x12);
		VL53L0X_RdByte(&vl53l0x_dev, 0x51, &len); // 高四位没法读出来
		printf("reg51 = 0x%x\n\r", len);

		Status = VL53L0X_WrWord(&vl53l0x_dev, 0x62, 0x9e12);
		VL53L0X_RdWord(&vl53l0x_dev, 0x62, &word);
		printf("reg61 = 0x%x\n\r", word);
	}

	if (vl53l0x_set_mode(&vl53l0x_dev, mode)) // 配置测量模式
	{
		printf("Mode Set Error!!!\r\n");
	}
	else
		printf("Mode Set OK!!!\r\n");

	while (1)
	{
		Status = vl53l0x_start_single_test(&vl53l0x_dev, &vl53l0x_data, buf);
		if (Status == VL53L0X_ERROR_NONE)
		{
			height_cm = vl53l0x_data.RangeMilliMeter / 10;
			uart_send_status_height(height_cm);
		}
		else
		{
			printf("Measurement Error!!!\r\n");
		}

		OLED_ShowString(1, 1, "Distance:");
		OLED_ShowNum(2, 7, Distance_data, 5);
		OLED_ShowString(2, 12, "mm");
		delay_ms(100);
	}
}
