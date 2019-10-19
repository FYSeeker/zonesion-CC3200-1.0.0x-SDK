/*********************************************************************************************
* 文件：beep.c
* 作者：Lixm 2017.10.17
* 说明：蜂鸣器驱动代码
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
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
#include "buzzer.h"

#define BEEP_GPIO_PIN   (1<<(4%8))
/*********************************************************************************************
* 名称：beep_init()
* 功能：继电器传感器初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void buzzer_init(void)
{
  
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
  PinTypeGPIO(PIN_59,PIN_MODE_0, false);
  
    
  GPIODirModeSet(GPIOA0_BASE, BEEP_GPIO_PIN, GPIO_DIR_MODE_OUT);
    
  

  GPIOPinWrite(GPIOA0_BASE, BEEP_GPIO_PIN, 0xff);
}

/*********************************************************************************************
* 名称：beep_control()
* 功能：继电器传感器控制函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void buzzer_on(void)
{
  GPIOPinWrite(GPIOA0_BASE, BEEP_GPIO_PIN, 0);
}
void buzzer_off(void)
{
  GPIOPinWrite(GPIOA0_BASE, BEEP_GPIO_PIN, 0xff);
}

