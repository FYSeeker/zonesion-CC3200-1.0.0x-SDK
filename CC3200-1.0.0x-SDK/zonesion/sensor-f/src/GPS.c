/*********************************************************************************************
* 文件：GPS.c
* 作者：zonesion
* 说明：驱动程序
* 修改：Chenkm 2017.01.04 增加了注释
* 注释：
*********************************************************************************************/
/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"
#include "uart_if.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*********************************************************************************************
* 全局变量
*********************************************************************************************/

static char _lat[16], _lng[16];
static char _gps_status = '0';

void uart1_handle(void);
static int gps_recv_ch(char ch);

/*********************************************************************************************
* 名称：uart1_init(void)
* 功能：串口1初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void gps_int(void)
{
  MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
  
  MAP_PinTypeUART(PIN_01, PIN_MODE_7);
  MAP_PinTypeUART(PIN_02, PIN_MODE_7);
  MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1), 
                          9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));
  UARTIntRegister(UARTA1_BASE,uart1_handle);
  UARTIntEnable(UARTA1_BASE,UART_INT_RX);
  UARTFIFODisable(UARTA1_BASE);
}
/*********************************************************************************************
* 名称：uart1_init(void)
* 功能：串口1初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart1_handle(void)
{
  if(UART_INT_RX == UARTIntStatus(UARTA1_BASE,true))
  { 
    char ch = UARTCharGet(UARTA1_BASE);
    gps_recv_ch(ch);
    
    UARTIntClear(UARTA1_BASE,UART_INT_RX);
  }
}
/*********************************************************************************************
* 名称：uart_send_char()
* 功能：串口发送字节函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart_send_char(unsigned char ch)
{
  MAP_UARTCharPut(UARTA1_BASE,ch);
}


/*********************************************************************************************
* 名称：gps_get()
* 功能：获取gps经纬度
* 参数：lat，lng 存放经纬度的缓存
* 返回：0：定位无效，1，定位有效
* 修改：
* 注释：
*********************************************************************************************/
int gps_get(char *lat, char *lng)
{
  if (lat != NULL) strcpy(lat, _lat);
  if (lng != NULL) strcpy(lng, _lng);
  if (_gps_status == '1' || _gps_status == '2') {
    return 1;
  }
  return 0;
}

static char* next(char *ip, char **ot)
{
  char *e = strchr(ip, ',');
  *ot = ip;
  if (e != NULL) {
    *e = '\0';
    return e+1;
  } 
  return NULL;
}

/*********************************************************************************************
* 名称：gps_recv_ch()
* 功能：
* 参数：ch
* 返回：0
* 修改：
* 注释：
*********************************************************************************************/
static int gps_recv_ch(char ch)
{
  static char f_idx = 0;
  static char tag[128];
  if (f_idx == 0) {
    tag[0] = tag[1];
    tag[1] = tag[2];
    tag[2] = tag[3];
    tag[3] = tag[4];
    tag[4] = tag[5];
    tag[5] = ch;
    if (memcmp(tag, "$GNGGA", 6) == 0) {
      f_idx = 6;
      return 0;
    }
  }
  if (f_idx) {
    tag[f_idx++] = ch;
    if (tag[f_idx-2]=='\r' && tag[f_idx-1]=='\n') {
      tag[f_idx] = '\0';
      char *p = tag, *pt;
      p = next(p, &pt);                                         //got $GPGGA
      p = next(p, &pt);                                         //utc time
      p = next(p, &pt);                                         //lat  ddmm.mmmm
      sprintf(_lat, pt);
      p = next(p, &pt);                                         //N/S
      if (*pt=='S') {
         char buf[16];
         sprintf(buf, "-%s", _lat);
         sprintf(_lat, buf);
      }
      p = next(p, &pt);                                         //lng  dddmm.mmmm
      sprintf(_lng, pt);
      p = next(p, &pt);                                         //E/W
      if (*pt == 'W') {
         char buf[16];
         sprintf(buf, "-%s", _lng);
         sprintf(_lng, buf);
      }
      p = next(p, &pt);                                         //st
      _gps_status = *pt;      
      f_idx = 0;
    }
  }
  return 0;
}

