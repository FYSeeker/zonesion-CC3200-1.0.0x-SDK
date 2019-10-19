/*********************************************************************************************
* 文件：au-uart.c
* 作者：xuzhy
* 说明：
*
*
*
* 修改：fuyou 增加透传驱动部分
* 注释：
*********************************************************************************************/
// Simplelink includes
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "simplelink.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"
#include "uart_if.h"

#define BUFF_SIZE 256

static char buff[BUFF_SIZE];
static int head  = 0;
static int real = 0;
/*********************************************************************************************
* 名称：uart0_handle
* 功能：at串口初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
static void fifo_put(char ch)
{
#if defined(CC3200_Serial)
    UARTIntDisable(UARTA1_BASE,UART_INT_RX);
#else
  UARTIntDisable(UARTA0_BASE,UART_INT_RX);
#endif
  int nh = head + 1;
  if (nh >= BUFF_SIZE) nh = 0;
  if (nh != real) {
    buff[head] = ch;
    head = nh;
  }
#if defined(CC3200_Serial)
    UARTIntEnable(UARTA1_BASE,UART_INT_RX);
#else
  UARTIntEnable(UARTA0_BASE,UART_INT_RX);
#endif
}
/*********************************************************************************************
* 名称：uart0_handle
* 功能：at串口初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
static int fifo_get(void)
{
#if defined(CC3200_Serial)
    UARTIntDisable(UARTA1_BASE,UART_INT_RX);
#else
    UARTIntDisable(UARTA0_BASE,UART_INT_RX);
#endif
  int r = -1;
  if (real != head) {
    r = buff[real]&0xff;
    real += 1;
    if (real >= BUFF_SIZE) real = 0;
  }
#if defined(CC3200_Serial)
    UARTIntEnable(UARTA1_BASE,UART_INT_RX);
#else
    UARTIntEnable(UARTA0_BASE,UART_INT_RX);
#endif
  return r;
}
/*********************************************************************************************
* 名称：uart0_handle
* 功能：at串口初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
#if defined(CC3200_Serial)
static void uart1_handle(void)
{
    if(UART_INT_RX == UARTIntStatus(UARTA1_BASE,true))
    {
        char ch  = UARTCharGet(UARTA1_BASE);
        fifo_put(ch);
        UARTIntClear(UARTA1_BASE,UART_INT_RX);
    }
}
#else
static void uart0_handle(void)
{
    if(UART_INT_RX == UARTIntStatus(UARTA0_BASE,true))
  { 
    char ch  = UARTCharGet(UARTA0_BASE);
    fifo_put(ch);
    UARTIntClear(UARTA0_BASE,UART_INT_RX);
  }
}
#endif
/*********************************************************************************************
* 名称：uart1_init(void)
* 功能：串口1初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
#if defined(CC3200_Serial)
void uart1_init(void)
{
  MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
  
  MAP_PinTypeUART(PIN_01, PIN_MODE_7);
  MAP_PinTypeUART(PIN_02, PIN_MODE_7);
  MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1), 
                          38400, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));
  UARTIntRegister(UARTA1_BASE,uart1_handle);
  UARTIntEnable(UARTA1_BASE,UART_INT_RX);
  UARTFIFODisable(UARTA1_BASE);
}
/*********************************************************************************************
* 名称：uart1_send_char()
* 功能：串口发送字节函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart1_send_char(unsigned char ch)
{
  MAP_UARTCharPut(UARTA1_BASE,ch);
}
#endif
/*********************************************************************************************
* 名称：at_uart_init
* 功能：at串口初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_uart_init(void)
{
#if defined(CC3200_Serial)
    uart1_init();
#else
  MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);

  // Configure PIN_55 for UART0 UART0_TX
  //
  MAP_PinTypeUART(PIN_55, PIN_MODE_3);

  //
  // Configure PIN_57 for UART0 UART0_RX
  //
  MAP_PinTypeUART(PIN_57, PIN_MODE_3);
  

  MAP_UARTConfigSetExpClk(UARTA0_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA0), 
                38400, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                 UART_CONFIG_PAR_NONE));

  UARTFIFODisable(UARTA0_BASE);
  UARTIntRegister(UARTA0_BASE,uart0_handle);
  UARTIntEnable(UARTA0_BASE,UART_INT_RX);
#endif
}
/*********************************************************************************************
* 名称：at_uart_write
* 功能：向at串口写一个字节
* 参数：ch：待发送字节
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_uart_write(char ch)
{
#if defined(CC3200_Serial)
    uart1_send_char(ch);
#else
    MAP_UARTCharPut(UARTA0_BASE, ch);
#endif
}
/*********************************************************************************************
* 名称：at_uart_read
* 功能：at读取1个字节
* 参数：
* 返回：<0没有数据，否则返回读取的字节
* 修改：
* 注释：
*********************************************************************************************/
int at_uart_read(void)
{
  return fifo_get();
}
