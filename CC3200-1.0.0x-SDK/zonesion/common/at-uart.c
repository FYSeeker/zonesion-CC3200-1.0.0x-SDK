/*********************************************************************************************
* �ļ���au-uart.c
* ���ߣ�xuzhy
* ˵����
*
*
*
* �޸ģ�fuyou ����͸����������
* ע�ͣ�
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
* ���ƣ�uart0_handle
* ���ܣ�at���ڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�uart0_handle
* ���ܣ�at���ڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�uart0_handle
* ���ܣ�at���ڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�uart1_init(void)
* ���ܣ�����1��ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�uart1_send_char()
* ���ܣ����ڷ����ֽں���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart1_send_char(unsigned char ch)
{
  MAP_UARTCharPut(UARTA1_BASE,ch);
}
#endif
/*********************************************************************************************
* ���ƣ�at_uart_init
* ���ܣ�at���ڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�at_uart_write
* ���ܣ���at����дһ���ֽ�
* ������ch���������ֽ�
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�at_uart_read
* ���ܣ�at��ȡ1���ֽ�
* ������
* ���أ�<0û�����ݣ����򷵻ض�ȡ���ֽ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int at_uart_read(void)
{
  return fifo_get();
}
