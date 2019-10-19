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

#define HWVER 180110
#if HWVER>= 180110
#define LED1_GPIO_PIN  (1<<(9%8))
#define LED2_GPIO_PIN  (1<<(22%8))
static unsigned int LED1_GPIO_BASE =  GPIOA1_BASE;
static unsigned int LED2_GPIO_BASE =  GPIOA2_BASE;
#else
#define LED1_GPIO_PIN  (1<<(11%8))
#define LED2_GPIO_PIN  (1<<(12%8))
static unsigned int LED1_GPIO_BASE =  GPIOA1_BASE;
static unsigned int LED2_GPIO_BASE =  GPIOA1_BASE;
#endif
void LEDInit(void)
{
#if HWVER>= 180110
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
    PinTypeGPIO(PIN_64,PIN_MODE_0, false);
    PinTypeGPIO(PIN_15,PIN_MODE_0, false);
    
    GPIODirModeSet(GPIOA1_BASE, LED1_GPIO_PIN, GPIO_DIR_MODE_OUT);
    GPIODirModeSet(GPIOA2_BASE, LED2_GPIO_PIN, GPIO_DIR_MODE_OUT);
    //GPIODirModeSet(GPIOA1_BASE, LED2_GPIO_PIN, GPIO_DIR_MODE_OUT);
 
#else
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    PinTypeGPIO(PIN_02,PIN_MODE_0, false);
    PinTypeGPIO(PIN_03,PIN_MODE_0, false);
    
    GPIODirModeSet(GPIOA1_BASE, LED1_GPIO_PIN|LED2_GPIO_PIN, GPIO_DIR_MODE_OUT);
    //GPIODirModeSet(GPIOA1_BASE, LED2_GPIO_PIN, GPIO_DIR_MODE_OUT);
    //GPIOPinWrite(GPIOA1_BASE, LED1_GPIO_PIN|LED2_GPIO_PIN, 0xff);
    //GPIOPinWrite(GPIOA1_BASE, LED2_GPIO_PIN, 0xff);
#endif
    GPIOPinWrite(LED1_GPIO_BASE, LED1_GPIO_PIN, 0xff);
    GPIOPinWrite(LED2_GPIO_BASE, LED2_GPIO_PIN, 0xff);
}
void LEDOn(unsigned int leds) 
{
  //unsigned char io = 0;
  //unsigned int gpio_base;
  
  if (leds & 0x01) {
    //io |= LED1_GPIO_PIN;
    GPIOPinWrite(LED1_GPIO_BASE, LED1_GPIO_PIN, 0);
  }
  if (leds & 0x02) {
    //io |= LED2_GPIO_PIN;
    GPIOPinWrite(LED2_GPIO_BASE, LED2_GPIO_PIN, 0);
  }
  //if (io != 0) {
  //  GPIOPinWrite(GPIOA1_BASE, io, 0);
  //}
}
void LEDOff(unsigned int leds) 
{
  //unsigned char io = 0;
  
  if (leds & 0x01) {
    //io |= LED1_GPIO_PIN;
    GPIOPinWrite(LED1_GPIO_BASE, LED1_GPIO_PIN, 0xff);
  }
  if (leds & 0x02) {
    //io |= LED2_GPIO_PIN;
    GPIOPinWrite(LED2_GPIO_BASE, LED2_GPIO_PIN, 0xff);
  }
  //if (io != 0) {
  //  GPIOPinWrite(GPIOA1_BASE, io, 0xff);
  //}
}