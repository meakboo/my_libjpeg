#ifndef __JPG_GPIO_H_
#define __JPG_GPIO_H_

#define ZYNQMP_GPIO_BASE		0xFF0A0000
#define ZYNQMP_MMAP_LEN			0x1000

#define ZYNQMP_EMIO_PIN_0		0
#define ZYNQMP_EMIO_PIN_1		1

#define ZYNQMP_BANK				3

#define ZYNQ_GPIO_DATA_LSW_OFFSET(BANK)	(0x000 + (8 * BANK))

#define ZYNQ_GPIO_DATA_RO_OFFSET(BANK)	(0x060 + (4 * BANK))

#define ZYNQ_GPIO_DATA_OFFSET(BANK)	(0x040 + (4 * BANK))

#define ZYNQ_GPIO_DIRM_OFFSET(BANK)	(0x204 + (0x40 * BANK))

#define ZYNQ_GPIO_OUTEN_OFFSET(BANK)	(0x208 + (0x40 * BANK))


#define ZYNQ_GPIO_MID_PIN_NUM 16
#define ZYNQ_GPIO_UPPER_MASK 0xFFFF0000


extern s32 jpeg_gpio_init(jpg_data_p jpg_data);
extern u8 jpeg_gpio_get_value(jpg_data_p jpg_data, u32 pin_num);
extern void jpeg_gpio_set_value(jpg_data_p jpg_data, u32 pin_num, u8 value);
extern void jpeg_gpio_set_dir(jpg_data_p jpg_data, u32 dir);

#endif //__JPG_GPIO_H_