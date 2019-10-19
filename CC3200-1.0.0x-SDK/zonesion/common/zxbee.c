#include <string.h>
#include <stdio.h>

#include "zxbee.h"
#include "zxbee-inf.h"

#define  ZXBEE_BUFFER_SIZE      1024


static char     msgTx[ZXBEE_BUFFER_SIZE];
/*********************************************************************************************
* 名称：ZXBeeBegin
* 功能：ZXBee指令合成开始
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeBegin(void)
{
  msgTx[0] = '{';
  msgTx[1] = '\0';
}
/*********************************************************************************************
* 名称：ZXBeeAdd
* 功能：添加一条数据到zxbee指令
* 参数：tag：传感器标识
        val：传感器值
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeAdd(char* pt, char*pv)
{
  int a = strlen((char*)msgTx);
  int b = strlen(pt);
  int c = strlen(pv);
  
  strcpy(&msgTx[a], pt);
  a += b;
  msgTx[a++] = '=';
  msgTx[a] = '\0';
  strcpy(&msgTx[a], pv);
  a += c;
  msgTx[a++] = ',';
  msgTx[a] = '\0';
}
/*********************************************************************************************
* 名称：ZXBeeEnd
* 功能：zxbee指令添加结束
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
char* ZXBeeEnd(void)
{
  int a = strlen(msgTx);
  msgTx[a-1] = '}';
  if (a > 2) {
    return msgTx;
  }
  return NULL;
}

/*********************************************************************************************
* 名称：ZXBeeDecodePackage
* 功能：zxbee指令解析
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
char* ZXBeeDecodePackage(char *buf, int len)
{
  
  if (buf[0] == '{' && buf[len-1] == '}') {
   
    char *p = &buf[1];
    char *ptag, *pval;
    
    buf[len-1] = '\0';
 
    ZXBeeBegin();
    while (p != NULL && *p != '\0') {
      ptag = p;
      p = strchr(p, '=');
      if (p != NULL) {
        *p++ = '\0';
        pval = p;
        p = strchr(p, ',');
        if (p != NULL) *p++ = '\0';
        if (ZXBeeSysCommandProc(ptag, pval)<0) {
#ifndef CC3200_Serial
                    ZXBeeUserProcess(ptag, pval);
#endif
        }
      }
    }
    return ZXBeeEnd();
  }
  return NULL;
}
