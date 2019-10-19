/*********************************************************************************************
* 文件：at.c
* 作者：xuzhy
* 说明：
*
*
*
* 修改：fuyou 增加透传驱动部分
* 注释：
*********************************************************************************************/
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
#include "ucfg.h"
#include "at.h"
#include "at-uart.h"
#include "zxbee-inf.h"

static char MAC[18];

#define AT_BUFF_SIZE    250
#define AT_BUFF_NUM     2
static char at_echo = 1;
static  int at_datalen = 0; //指示用于接收发送的数据长度
static  int at_recvdata = 0;
static char atbuff[AT_BUFF_NUM][AT_BUFF_SIZE];
static char bufferbit = 0;
/*********************************************************************************************
* 名称：at_quebuffer_get
* 功能：at缓存申请
* 参数：
* 返回：成功返回缓存地址，失败返回NULL
* 修改：
* 注释：
*********************************************************************************************/
char* at_quebuffer_get()
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (((bufferbit>>i) & 1) == 0) {
			bufferbit |= 1<<i;
			return atbuff[i];
		}
	}
	return NULL;
}

/*********************************************************************************************
* 名称：at_quebuffer_put
* 功能：at缓存释放
* 参数：buf：待释放缓存地址
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_quebuffer_put(char *buf)
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (atbuff[i] == buf){
			bufferbit &= ~(1<<i);
			return;
		}
	}
}
/*********************************************************************************************
* 名称：_at_get_ch
* 功能：at串口接收到1个字节处理函数
* 参数：ch：接收到的字节
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
static int _at_get_ch(char ch)
{
  static char idx = 0;
  static char *pbuf = NULL;
  if (pbuf == NULL) {
    pbuf = at_quebuffer_get();
    if (pbuf == NULL) { 
      return -1;
    }
  }
  if (at_datalen == 0) {
    if (at_echo) {
      at_uart_write(ch);
    }
    if (idx < AT_BUFF_SIZE-1) {
      pbuf[idx++] = ch;
      if (idx >= 2 && pbuf[idx-2]=='\r' && pbuf[idx-1]=='\n'){
        idx -= 2;
        pbuf[idx] = '\0';
        if (idx > 0) {
          /* at指令接收完成 */
           at_proc(pbuf);
           at_quebuffer_put(pbuf);
          pbuf = NULL;
          idx = 0;
        }
      }
    } else {
      /*缓存溢出丢弃所有数据*/
      idx = 0;
    }
  } else {
    pbuf[at_recvdata++] = ch;
    if (at_recvdata == at_datalen) {
      /*数据接收完成*/
      at_proc(pbuf);
      at_quebuffer_put(pbuf);
      at_datalen = 0;
      pbuf = NULL;
    }
  }
  return idx;
}
/*********************************************************************************************
* 名称：at_response_buf
* 功能：at接口发送一段数据
* 参数：s：待发送数据地址
*      len：待发送数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response_buf(char *s, int len)
{
  for (int i=0; i<len; i++) {
    at_uart_write(s[i]);
  }
}
/*********************************************************************************************
* 名称：at_response
* 功能：at接口发送一段字符串
* 参数：s：待发送字符串
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response(char *s)
{
  at_response_buf(s, strlen(s));
}
/*********************************************************************************************
* 名称：at_notify_data
* 功能：通过at指令输出接收到的数据s
* 参数：buf：接收到的zxbee数据
*       len： 数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_notify_data(char *buf, int len)
{
  char temp[64] = {0};
  short n = sprintf(temp,"+RECV:%u\r\n", len);
  at_response_buf(temp, n);
  at_response_buf(buf, len);
}
/*********************************************************************************************
* 名称：at_check
* 功能：at命令串口读取查询函数
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int at_check(void)
{
  static char buf[256];
  int r = 0;
  int rlen = 0;
  do {
    rlen = 0;
    do {
      r = at_uart_read();
      if (r >= 0 && rlen<256) {
        buf[rlen++] = r;
      }
    } while (r>=0);
    for (int i=0; i<rlen; i++) {
       r = _at_get_ch(buf[i]);
    }
  }while (rlen>0);
  
  return r;
}

/*********************************************************************************************
* 名称：ATCommandInit
* 功能：at接口初始
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ATCommandInit(void)
{
  at_uart_init();
  
   _u8 macAddressVal[SL_MAC_ADDR_LEN];
   _u8 macAddressLen = SL_MAC_ADDR_LEN;
   sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(_u8 *)macAddressVal);
   sprintf(MAC, "%02X:%02X:%02X:%02X:%02X:%02X",
              macAddressVal[0], macAddressVal[1], macAddressVal[2],
              macAddressVal[3], macAddressVal[4], macAddressVal[5]);

    at_response("+HW:CC3200\r\n");
    at_response("+RDY\r\n");
}
/*********************************************************************************************
* 名称：at_proc
* 功能：at命令处理
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_proc(char *p_msg)
{
  char tempbuf[128] = {0};
  if (at_recvdata != 0) {
    /*处理发送数据*/
    at_response(ATOK);
    
    sendMessage(p_msg, at_recvdata);
   
    sprintf(tempbuf,"+SEND:%u\r\n", at_recvdata);
    at_response(tempbuf);
    at_recvdata = 0;
  } else {
    int msg_size = strlen(p_msg);
    for (int i=0; i<msg_size; i++){
      if (p_msg[i] == '?' || p_msg[i] == '=') break;
      p_msg[i] = toupper(p_msg[i]);
    }
    
    if (strcmp(p_msg, "AT") == 0) {
      at_response(ATOK);
    }
    else if (strcmp(p_msg, "ATE1") == 0) {
      at_echo = 1;
      at_response(ATOK);
    }
    else if (strcmp(p_msg, "ATE0") == 0) {
      at_echo = 0;
      at_response(ATOK);
    }  
    else if (memcmp(p_msg, "AT+MAC?", 7) == 0) {
       _u8 macAddressVal[SL_MAC_ADDR_LEN];
       _u8 macAddressLen = SL_MAC_ADDR_LEN;
       sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(_u8 *)macAddressVal);
       sprintf(tempbuf,"+MAC:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
              macAddressVal[0], macAddressVal[1], macAddressVal[2],
              macAddressVal[3], macAddressVal[4], macAddressVal[5]);
       at_response(tempbuf);
       at_response(ATOK);
    }
    else if (memcmp(p_msg, "AT+SSID", 7) == 0) {
      if (p_msg[7] == '=') { //设置ssid
        if (p_msg[8] == '"' && p_msg[msg_size-1] == '"') {
          int slen = msg_size - 10;
          if (slen < sizeof cfg.ssid) {
            p_msg[msg_size-1] = '\0';
            strcpy(cfg.ssid, &p_msg[9]);
            at_response(ATOK);
          } else {
             at_response(ATERROR);
          }
        } else {
           at_response(ATERROR);
        }
      } else if (p_msg[7] == '?') { //读取ssid
        sprintf(tempbuf,"+SSID:\"%s\"\r\n", cfg.ssid);
        at_response(tempbuf);
        at_response(ATOK);
      } else { //错误参数
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+KEYTYPE", 10) == 0) {
      if (p_msg[10] == '=') {
        int t = atoi(&p_msg[11]);
        if (t >= 0 && t<=2) {
          cfg.stype = t;
          at_response(ATOK);
        } else {
          at_response(ATERROR);
        }
      } else if (p_msg[10] == '?') {
        sprintf(tempbuf,"+KEYTYPE:%u\r\n", cfg.stype);
        at_response(tempbuf);
        at_response(ATOK);
      } else {
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+KEY", 6) == 0) {
      if (p_msg[6] == '=') {
        if (p_msg[7] == '"' && p_msg[msg_size-1] == '"') {
             p_msg[msg_size-1] = '\0';
             int slen = msg_size - 9;
             if (slen < sizeof cfg.skey) {
               strcpy(cfg.skey, &p_msg[8]);
               at_response(ATOK);
             } else {
               at_response(ATERROR);
             }
        } else {
          at_response(ATERROR);
        }
      } else if (p_msg[6] == '?') {
        sprintf(tempbuf,"+KEY:\"%s\"\r\n", cfg.skey);
        at_response(tempbuf);
        at_response(ATOK);
      } else {
        at_response(ATERROR);
      }
    }

    else if (memcmp(p_msg, "AT+SIP", 6) == 0) {
      if (p_msg[6] == '=') {
         char *e[3];
         char *po;
         
         e[0] = strchr(&p_msg[7], '.');
         if (e[0] == NULL) goto _e1;
         e[1] = strchr(e[0]+1, '.');
         if (e[1] == NULL) goto _e1;
         e[2] = strchr(e[1]+1, '.');
         if (e[2] == NULL) goto _e1;
         *e[0] = '\0'; *e[1] = '\0'; *e[2] = '\0';
         
         po = strchr(e[2]+1, ',');
         if (po != NULL) {
            cfg.sport = atoi(po+1);
         }
         
         cfg.sip[0] = atoi(&p_msg[7]);
         cfg.sip[1] = atoi(e[0]+1);
         cfg.sip[2] = atoi(e[1]+1);
         cfg.sip[3] = atoi(e[2]+1);
         
         at_response(ATOK);
         goto _end;
      _e1:
        at_response(ATERROR);
      _end:
        //None
        while (0);
      }else if (p_msg[6] == '?') {
        sprintf(tempbuf,"+SIP:%u.%u.%u.%u\r\n", cfg.sip[0],cfg.sip[1],cfg.sip[2],cfg.sip[3]);
        at_response(tempbuf);
        at_response(ATOK);
      } else {
         at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+ENVSAVE", 10) == 0) {
      ucfg_save();
      at_response(ATOK);
    } 
    else if (memcmp(p_msg, "AT+HW?", 6) == 0) {
      at_response("+HW:CC3200\r\n");
      at_response(ATOK);
    }
    else if (memcmp(p_msg, "AT+RESET", 8)==0) {
       at_response(ATOK);
       sl_Stop(30);  
       MAP_PRCMHibernateIntervalSet(330);
       MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
       MAP_PRCMHibernateEnter();
    }
    else if (memcmp(p_msg, "AT+SEND=", 8) == 0) {
      int len = atoi(&p_msg[8]);
      if (len > 0 && len<=AT_BUFF_SIZE) {
        at_response(">");
        at_datalen = len;
      } else {
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+IP", 5) == 0) {
      if (p_msg[5] == '?') {
        char * getOurIpAddr(void);
        at_response("+IP:");
        at_response(getOurIpAddr());
        at_response("\r\n");
        at_response(ATOK);
      }
    }
    else {
      int user_at_proc(char *msg);
        if (user_at_proc(p_msg)<0)
        {
            at_response(ATERROR);
        }
    }
  }
  
}


void AT_ReportedLinkStatus()
{
    
}

