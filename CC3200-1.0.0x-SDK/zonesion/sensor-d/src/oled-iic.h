/*********************************************************************************************
* 文件：oled_iic.h
* 作者：zonesion
* 说明：oled_iic头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _OLED_IIC_H_
#define _OLED_IIC_H_

#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "prcm.h"
#include "pinmux.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "pin.h"
#include <math.h>
#include <stdio.h>
/*********************************************************************************************
* 外部原型函数
*********************************************************************************************/
void oled_iic_delay_us(unsigned int i);
void oled_iic_init(void);
void oled_iic_start(void);
void oled_iic_stop(void);
void oled_iic_send_ack(int ack);
int oled_iic_recv_ack(void);
unsigned char oled_iic_write_byte(unsigned char data);
unsigned char oled_iic_read_byte(unsigned char ack);
void delay(unsigned int t);

#endif 