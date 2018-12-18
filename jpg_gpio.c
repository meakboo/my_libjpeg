#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "my_jpeg.h"
#include "jpg_gpio.h"

s32 jpeg_gpio_init(jpg_data_p jpg_data)
{
	jpg_data->gpio_mem = (void *) mmap(NULL, ZYNQMP_MMAP_LEN,  PROT_READ | PROT_WRITE ,\
					MAP_SHARED,  jpg_data->mem_fd, ZYNQMP_GPIO_BASE) ;

	if((!jpg_data->gpio_mem) || (jpg_data->gpio_mem == (void *)-1))
	{
		ERRDEBUG("gpio mmap failed\n");
		return -1;
	}

	return 0;
}

u8 jpeg_gpio_get_value(jpg_data_p jpg_data, u32 pin_num)
{
	u32 temp = 0;

	temp = *(u32 *)(jpg_data->gpio_mem + ZYNQ_GPIO_DATA_OFFSET(ZYNQMP_BANK));

	printf("temp = %x\n", temp);

	return ((temp >> pin_num) & 0x1);
}

void jpeg_gpio_set_dir(jpg_data_p jpg_data, u32 dir)
{
	*(u32 *)(jpg_data->gpio_mem + ZYNQ_GPIO_DIRM_OFFSET(ZYNQMP_BANK)) = dir;
	*(u32 *)(jpg_data->gpio_mem + ZYNQ_GPIO_OUTEN_OFFSET(ZYNQMP_BANK)) = dir;
}

void jpeg_gpio_set_value(jpg_data_p jpg_data, u32 pin_num, u8 value)
{
	value = !!value;

	value = ~(1 << (pin_num + ZYNQ_GPIO_MID_PIN_NUM)) & ((value << pin_num) | ZYNQ_GPIO_UPPER_MASK);

	*(u32 *)(jpg_data->gpio_mem + ZYNQ_GPIO_DATA_LSW_OFFSET(ZYNQMP_BANK))  = value;

	return;
}

