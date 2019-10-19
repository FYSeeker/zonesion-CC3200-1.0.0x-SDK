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
//gpio8, gpio25

#define KEY1_GPIO_PIN  (1<<(8%8))  //gpio8 --- pin63
#define KEY2_GPIO_PIN  (1<<(25%8)) //gpio25 --- pin21

void key_init(void)
{
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    PinTypeGPIO(PIN_63,PIN_MODE_0, false);
    PinTypeGPIO(PIN_21,PIN_MODE_0, false);
    
    GPIODirModeSet(GPIOA1_BASE, KEY1_GPIO_PIN, GPIO_DIR_MODE_IN);
    GPIODirModeSet(GPIOA3_BASE, KEY2_GPIO_PIN, GPIO_DIR_MODE_IN); //手册上此io只能做输出??
}
unsigned char get_key_status(void)
{
  long k1 = GPIOPinRead(GPIOA1_BASE, KEY1_GPIO_PIN);
  long k2 = GPIOPinRead(GPIOA3_BASE, KEY2_GPIO_PIN);
  char ret = 0;
  if (k1 == 0) ret |= 1;
  if (k2 != 0) ret |= 2;  //k2状态在核心板反转
  return ret;
}