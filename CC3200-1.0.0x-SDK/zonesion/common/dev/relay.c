/*********************************************************************************************
* 文件：relay.c
* 作者：Lixm 2017.10.17
* 说明：继电器驱动代码
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "relay.h"

/*********************************************************************************************
* 名称：relay_init()
* 功能：继电器传感器初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void relay_init(void)
{
  PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);      //使能时钟
  PinTypeGPIO(PIN_08,PIN_MODE_0,0);                           //选择引脚为GPIO模式（gpio17）
  GPIODirModeSet(GPIOA2_BASE, G17_UCPINS, GPIO_DIR_MODE_OUT);   //设置GPIO17为输出模式
  
  
  PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);      //使能时钟
  PinTypeGPIO(PIN_18,PIN_MODE_0,0);                           //选择引脚为GPIO模式（gpio28）
  GPIODirModeSet(GPIOA3_BASE, G28_UCPINS, GPIO_DIR_MODE_OUT);   //设置GPIO28为输出模式
  
  GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0xFF);
  GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0xFF);                  //初始化断开继电器
}

signed int relay_on(char cmd)
{
  if(cmd & 0x01){
    GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0x00);
   
  }
  if(cmd & 0x02){
    GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0x00);
    
  }
   return 0;
}


signed int relay_off(char cmd)
{
  if(cmd & 0x01){
    GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0xff);
    
  }
  if(cmd & 0x02){
    GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0xff);
    
  }
  return 0;
}

void relay_control(char cmd)
{
  relay_on(cmd);
  relay_off((~cmd)&0x03);
}
