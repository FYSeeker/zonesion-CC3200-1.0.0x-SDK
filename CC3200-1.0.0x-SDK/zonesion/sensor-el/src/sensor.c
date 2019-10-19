/*********************************************************************************************
* 文件：sensor.c
* 作者：Xuzhy 2018.5.16
* 说明：xLab Sensor-EL传感器程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "udma_if.h"
#include "common.h"
#include "hw_ints.h"
#include "hw_types.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "systick_if.h"
#include "gpio.h"
#include "hw_memmap.h"
#include "pin.h"
#include "prcm.h"
#include "adc.h"
#include "relay.h"
#include "math.h"
#include "sensor.h"
#include "oled.h"
#include "relay.h"
#include "key.h"
#include "buzzer.h"
#include "7941w.h"
#include "zxbee.h"
#include "zxbee-inf.h"
/*********************************************************************************************
* 宏定义
*********************************************************************************************/
#define KEY     "\xFF\xFF\xFF\xFF\xFF\xFF"

/*********************************************************************************************
* 全局变量
*********************************************************************************************/
static uint8 D1 = 2;                                            // 继电器初始状态为全关
static char A0[16];                                             // A0存储卡号
static uint8 A1 = 0;                                            // 卡类型 0:id卡 1：充值卡
static uint32 A2 = 0;                                           // 卡余额
static uint32 A3 = 10000;                                       // 设备余额，初始值为100元 
static uint32 A4 = 0;                                           // 设备当前消费
static uint32 A5 = 0;                                           // 设备累计消费
static uint32 V1 = 0; 
static uint32 V2 = 0; 
static int work_mod = 2;                                        // 1 设备充值模式, 2 读卡模式,上位机可以给卡片充值
/*********************************************************************************************
* 名称：sensorInit()
* 功能：传感器硬件初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorInit(void)
{
  RFID7941Init();
  
  key_init();
 
  relay_init();
  
  relay_control(D1);
  buzzer_init();
  OLED_Init();
  
  OLED_Clear();
  OLED_ShowCHinese(21,2,4);
  OLED_ShowCHinese(21+13,2,5);
  OLED_ShowCHinese(21+13+13,2,12);
  
  OLED_ShowCHinese(21,0,8);
  OLED_ShowCHinese(21+13,0,9);
  OLED_ShowCHinese(21+13+13,0,12);
  if(work_mod == 1){
    OLED_ShowCHinese(21+13*3,0,6);
    OLED_ShowCHinese(21+13*4,0,7);
  }
  else{
    OLED_ShowCHinese(21+13*3,0,10);
    OLED_ShowCHinese(21+13*4,0,11);
  }
  char buf[16];
  sprintf(buf, "%d.%d", A3/100, A3%100);
  OLED_ShowString(21+13*3,2,(unsigned char *)buf,12); 
}
/*********************************************************************************************
* 名称：sensorLinkOn()
* 功能：传感器节点入网成功调用函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
}
/*********************************************************************************************
* 名称：sensorUpdate()
* 功能：处理主动上报的数据
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  if (A3 > 0) {
    A4 = 1 + rand()%500;                                              // 设备本次消费扣款金额，通过随机数产生
    if (A4 > A3) A4 = A3;
    
    A3 = (A3 - A4);                                             // 设备余额
    A5 = (A5 + A4);                                             // 设备累计消费金额
    
    ZXBeeBegin();
    sprintf(p, "%d.%d", A3/100, A3%100);                      // 气表余额
    ZXBeeAdd("A3", p);
    sprintf(p, "%d.%d", A4/100, A4%100);                      // 气表本次消费扣款金额
    ZXBeeAdd("A4", p);
    sprintf(p, "%d.%d", A5/100, A5%100);                      // 气表累积消费金额
    ZXBeeAdd("A5", p);
	  
    p = ZXBeeEnd();                                             // 智云数据帧格式包尾
    if (p != NULL) {												
      ZXBeeInfSend(p, strlen(p));	                            // 将需要上传的数据进行打包操作，并通过zb_SendDataRequest()发送到协调器
    }
    sprintf(p, "%d.%d  ", A3/100, A3%100);
    OLED_ShowString(21+13*3,2,(unsigned char *)p,12);           // oLED显示气表余额
    if(A3 == 0) {   //余额不足扣费，继电器断开（模拟气表设备断开）
      D1 &= ~2;
      relay_control(D1);
    }
  }
}
/*********************************************************************************************
* 名称：sensorCheck()
* 功能：传感器监测
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorCheck(void)
{ 
  char buff[16];
  
  ZXBeeBegin();
  // 按键处理
  uint8 k = get_key_status();
  static uint8 last_key = 0;
  if (k & 0x01 == 0x01) {
    if ((last_key & 0x01) == 0) {
      if (work_mod == 1) {
        OLED_ShowCHinese(21+13*3,0,10);
        OLED_ShowCHinese(21+13*4,0,11);
        work_mod = 2;
      } else {
        OLED_ShowCHinese(21+13*3,0,6);
        OLED_ShowCHinese(21+13*4,0,7);
        work_mod = 1;
      }
    }
  } else if ((k & 0x02) == 0x02) {
    if ((last_key & 0x02) == 0) {
      //模拟消费
      if (A3 > 0) {
        A4 = rand()%500;                                          // 设备本次消费扣款金额，通过随机数产生
        if (A4 > A3) A4 = A3;
        A3 = A3 - A4;
        A5 = A5 + A4;
        sprintf(buff, "%d.%d", A3/100,A3%100);
        ZXBeeAdd("A3", buff);
        sprintf(buff, "%d.%d", A4/100,A4%100);
        ZXBeeAdd("A4", buff);
        sprintf(buff, "%d.%d", A5/100,A5%100);
        ZXBeeAdd("A5", buff);

        sprintf(buff, "%d.%d  ", A3/100, A3%100);
        OLED_ShowString(21+13*3,2,(unsigned char *)buff,12);           // oLED显示气表余额
        if(A3 == 0) {   //余额不足扣费，继电器断开（模拟气表设备断开）
          D1 &= ~2;
          relay_control(D1);
        }
      }
    }
  }
  last_key = k;
  //  按键处理完毕
  
  // 读卡处理
  static char status = 1;
  static char last_cid[7];
  static char card_cnt = 0;                                     // 卡片检测计数
  static char cid[7];
  static char write = 0;
  
  if (status == 1) {                                            // 检测125k读卡结果
    buzzer_off();
    if (RFID7941CheckCard125kHzRsp(cid) > 0) {
      if (memcmp(last_cid, cid, 5) != 0) {
        sprintf(A0, "%02X%02X%02X%02X%02X", cid[0],cid[1],cid[2],cid[3],cid[4]);
        ZXBeeAdd("A0", A0);
        A1 = 0;
        sprintf(buff, "%d", A1);
        ZXBeeAdd("A1", buff);
        memcpy(last_cid, cid, 5);
        card_cnt = 5;
        buzzer_on(); 
      } else { //同一张卡片
        card_cnt = 5;
      }
      RFID7941CheckCard125kHzReq();
    } else {
      if (card_cnt > 0) {
        card_cnt -= 1;
      }
      if (card_cnt == 0) {
        memset(last_cid, 0, sizeof last_cid);
        RFID7941CheckCard1356MHzReq();                          // 检测13.56M卡片
        status = 2;
      } else {
        RFID7941CheckCard125kHzReq();
      }
    }
  } else if (status == 2) {                                     // 13.56卡片检测处理
    buzzer_off();
    if (RFID7941CheckCard1356MHzRsp(cid) > 0) {
      if (memcmp(last_cid, cid, 4) != 0) {
        RFID7941ReadCard1356MHzReq(8, KEY);
        status = 3;
      } else {
        card_cnt = 5;                                           // 刷新卡片计算
        if (work_mod == 1) {                                    // 充值设备
          if (write == 3 && A2 > 0) {
            memset(buff, 0, 16);
            RFID7941WriteCard1356MHzReq(8, KEY, buff);
            status = 4;
          } else {
            RFID7941CheckCard1356MHzReq();
          }
        } else
        if (work_mod == 2) {                                    // 读卡模式
          int money;
          if (V1 != 0) {                                        // 充值
            money = A2 + V1;    
            write = 1;
          } else if (V2 != 0) {
            money = A2 - V2;
            write = 2;
          }
          if (V1 != 0 || V2 != 0) {
            memset(buff, 0, 16);
            buff[12] = (money>>24) & 0xff;
            buff[13] = (money>>16) & 0xff;
            buff[14] = (money>>8) & 0xff;
            buff[15] = money&0xff;
            RFID7941WriteCard1356MHzReq(8, KEY, buff);
            status = 5;
          } else {
            RFID7941CheckCard1356MHzReq();
          }
        } else {
          RFID7941CheckCard1356MHzReq();
        }
      }
    } else {
      if (card_cnt > 0) {
        card_cnt -= 1;
      }
      if (card_cnt == 0) {
        memset(last_cid, 0, sizeof last_cid);
        RFID7941CheckCard125kHzReq();
        status = 1;
        A2 = 0;                                                 // 余额清零
      } else {
        RFID7941CheckCard1356MHzReq();
      }
    }
  } else if (status == 3) {                                     // 处理卡片读取结果
    if (RFID7941ReadCard1356MHzRsp(buff) > 0) {                 // 读取到余额，保存当前卡片id
      memcpy(last_cid, cid, 4);
      card_cnt = 5;      
      sprintf(A0, "%02X%02X%02X%02X", cid[0],cid[1],cid[2],cid[3]);  
      int money = ((uint32)buff[12]<<24) | ((uint32)buff[13]<<16) | (buff[14]<<8) | (buff[15]);
      A2 = money;
      if (work_mod == 1) {                                      // 设备充值模式
        if (money > 0) {
          buff[12] = buff[13] = buff[14] = buff[15] = 0;
          write = 3;                                            // 再次检测卡片后通过write = 3 来写卡
          RFID7941CheckCard1356MHzReq();
          status = 2;
        } else {
          buzzer_on();                          
          RFID7941CheckCard1356MHzReq();
          status = 2;
        }
      } else {  //work_mod == 2;
        ZXBeeAdd("A0", A0);
        ZXBeeAdd("A1", "1");
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        V1 = 0;
        V2 = 0;
        buzzer_on();                          //
        RFID7941CheckCard1356MHzReq();
        status = 2;
      }
    } else {
      RFID7941CheckCard1356MHzReq();                            // 重新检测卡片
      status = 2;
    }
  } else if (status == 4) {                                     // 处理充值设备写卡结果
 
      if (RFID7941WriteCard1356MHzRsp() > 0) {
        //充值设备成功
        A3 += A2;
        A2 = 0;
        sprintf(buff, "%d.%d   ", A3/100,A3%100);
        OLED_ShowString(21+13*3,2,(unsigned char *)buff,12);
        
        write = 0;
        D1 |= 2;
        relay_control(D1);
          
        buzzer_on();
      } else { //写卡失败
        //memset(last_cid, 0, sizeof last_cid);
        //card_cnt = 0;
      }
      RFID7941CheckCard1356MHzReq();
      status = 2;
  } else if (status == 5) {                                     // 卡片充值扣费结果检测
    if (RFID7941WriteCard1356MHzRsp() > 0) {
      if (write == 1) {
        ZXBeeAdd("V1", "1");
        A2 += V1;
        V1 = 0;
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        write = 0;
      } else if (write == 2) {
        ZXBeeAdd("V2", "1");
        A2 -= V2;
        V2 = 0;
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        write  = 0;
      }
      buzzer_on();
    }
    RFID7941CheckCard1356MHzReq();
    status = 2;
  }
  
  char *p = ZXBeeEnd();
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}
/*********************************************************************************************
* 名称：sensorControl()
* 功能：传感器控制
* 参数：cmd - 控制命令
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorControl(unsigned char cmd)
{
  // 根据cmd参数处理对应的控制程序
  relay_control(cmd);
}
/*********************************************************************************************
* 名称：ZXBeeUserProcess()
* 功能：解析收到的控制命令
* 参数：*ptag -- 控制命令名称
*       *pval -- 控制命令参数
* 返回：<0:不支持指令，>=0 指令已处理
* 修改：
* 注释：
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  char buf[32];
  // 将字符串变量pval解析转换为整型变量赋值
  val = atoi(pval);	
  
  if (0 == strcmp("V1", ptag)) {
    float v = atof(pval);
    V1 = (int)(v * 100);
    
  }
  if (0 == strcmp("V2", ptag)) {
    float v = atof(pval);
    V2 = (int)(v * 100);
    if (V2 > A2) {
      V2 = 0;
      ZXBeeAdd("V2", "0");
    }
  }
  if (0 == strcmp("V3", ptag)) {
    float v = atof(pval); 
    A3 =  A3 + (int)(v * 100);
    ZXBeeAdd("V3", "1");  
    D1 |= 2;
    relay_control(D1);
  }
  if (0 == strcmp("V4", ptag)) {
    float v = atof(pval);
    unsigned int x = (unsigned int)(v * 1000);
    if (A3 >= x) {
        A3 =  A3 - (long)(v * 100);
        ZXBeeAdd("V4", "1");
        if (A3 == 0) {
                D1 &= ~2;
                relay_control(D1);
        }
    } else {
        ZXBeeAdd("V4", "0");
    }
  }

  // 控制命令解析
  if (0 == strcmp("CD1", ptag)){                                // 对D1的位进行操作，CD1表示位清零操作
    D1 &= ~val;
    sensorControl(D1);
  }
  if (0 == strcmp("OD1", ptag)){                                // 对D1的位进行操作，OD1表示位置一操作
    D1 |= val;
    sensorControl(D1);
  }
  if (0 == strcmp("D1", ptag)){                                 
    if (0 == strcmp("?", pval)){                                
      sprintf(buf, "%u", D1);                         
      ZXBeeAdd("D1", buf);
    } 
  }
  return 0;
}
/*********************************************************************************************
* 名称：sensorLoop()
* 功能：定时触发功能
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorLoop(void)
{
  static unsigned long ct_update = 0;
  static unsigned long ct_check = 0;
  
  if (t4exp(ct_update)) {
    sensorUpdate();
    ct_update = t4ms()+30*1000;
  }
  if (t4exp(ct_check)) {
    sensorCheck();
    ct_check = t4ms()+100;
  }
}
