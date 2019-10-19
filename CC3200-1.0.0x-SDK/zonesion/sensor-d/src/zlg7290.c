/*********************************************************************************************
* 文件：zlg7290.c
* 作者：zonesion
* 说明：zlg7290驱动程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/

#include <math.h>
#include <stdio.h>
#include "zlg7290.h"
#include "iic2.h"
#include "uart_if.h"
#include "systick_if.h"

#define G59_UCPINS              ( 1 << (59%8))

#define delay_ms(x)             UTUtilsDelay((x)*1000)
/*********************************************************************************************
* 全局变量
*********************************************************************************************/
extern void GPIO59_handle(void);

/*********************************************************************************************
* 名称：zlg7290_init()
* 功能：zlg7290初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void zlg7290_init(void)
{
  iic2_init();		                                            //I2C初始化

  zlg7290_write_data(SCANNUM, 3); 
  
  PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);      //使能时钟
  PinTypeGPIO(PIN_59,PIN_MODE_0,false);                        //选择引脚为GPIO模式（gpio4）
  PinConfigSet(PIN_59,PIN_STRENGTH_2MA,PIN_TYPE_STD_PU);
  GPIODirModeSet(GPIOA0_BASE, G59_UCPINS, GPIO_DIR_MODE_IN);    //设置GPIO4为输入模式
  GPIOIntTypeSet(GPIOA0_BASE, G59_UCPINS, GPIO_FALLING_EDGE);
  //设置中断处理函数
  GPIOIntRegister(GPIOA0_BASE,GPIO59_handle);
  //中断使能
  GPIOIntEnable(GPIOA0_BASE,GPIO_INT_PIN_4);
}

/*********************************************************************************************
* 名称：zlg7290_read_reg()
* 功能：zlg7290读取寄存器
* 参数：cmd -- 寄存器地址
* 返回：data 寄存器数据
* 修改：
* 注释：
*********************************************************************************************/
unsigned char zlg7290_read_reg(unsigned char cmd)
{
  unsigned char data = 0; 				        //定义数据
  
  
  iic2_read_buf(ZLG7290ADDR>>1, cmd, (char*)&data, 1);
  delay_ms(1);
  return data;						        //返回数据
}

/*********************************************************************************************
* 名称：zlg7290_write_data()
* 功能：zlg7290写寄存器
* 参数：cmd -- 寄存器地址；data 寄存器数据
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void zlg7290_write_data(unsigned char cmd, unsigned char data)
{
  
  iic2_write_buf(ZLG7290ADDR>>1, cmd, (char*)&data, 1);
  delay_ms(1);
}


/*********************************************************************************************
* 名称：get_keyval()
* 功能：获取按键键值
* 参数：
* 返回：按键键值
* 修改：
* 注释：
*********************************************************************************************/
unsigned char zlg7290_get_keyval(void)
{
  unsigned char key_num = 0;
  key_num = zlg7290_read_reg(KEY_REG);

  if(key_num == 5)
    return UP;
  if(key_num == 13)
    return LEFT;
  if(key_num == 21)
    return DOWN;
  if(key_num == 29)
    return RIGHT;
  if(key_num == 37)
    return CENTER;
  return 0;
}


/*********************************************************************************************
* 名称：zlg7290_download()
* 功能：下载数据并译码
* 参数：addr    ：取值0～7，显示缓存DpRam0～DpRam7的编号
*       dp      ：是否点亮该位的小数点，0－熄灭，1－点亮
*       flash   ：控制该位是否闪烁，0－不闪烁，1－闪烁, 2-不操作
*       dat     ：取值0～31，表示要显示的数据 
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void zlg7290_download(unsigned char addr, unsigned dp, unsigned char flash, unsigned char dat)
{
  unsigned char cmd0;
  unsigned char cmd1;
  cmd0 = addr & 0x0F;
  cmd0 |= 0x60; 
  cmd1 = dat & 0x1F;
  if ( dp == 1 )
    cmd1 |= 0x80;
  if ( flash == 1 )
    cmd1 |= 0x40;
  //zlg7290_set_smd(cmd0,cmd1);
  char cmd[2];
  cmd[0] = cmd0;
  cmd[1] = cmd1;
  iic2_write_buf(ZLG7290ADDR>>1, CMDBUF0, cmd, 2);
  delay_ms(1);
}



/*********************************************************************************************
* 名称：segment_display()
* 功能：数码管显示数字
* 参数：num -- 数据（最大9999）
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void segment_display(unsigned int num)
{
  static unsigned char h = 0,j = 0,k = 0,l = 0;
  if(num > 9999)
    num = 0;
  h = num % 10;
  j = num % 100 /10;
  k = num % 1000 / 100;
  l = num /1000;
  zlg7290_download(0,0,0,l);
  zlg7290_download(1,0,0,k);
  zlg7290_download(2,0,0,j);
  zlg7290_download(3,0,0,h);
}



/*********************************************************************************************
* 名称：displa_off()
* 功能：数码管显示数字
* 参数：num -- 数据（最大9999）
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void display_off(void)
{
  zlg7290_download(0,0,0,31);  
  zlg7290_download(1,0,0,31);
  zlg7290_download(2,0,0,31);
  zlg7290_download(3,0,0,31);
}
