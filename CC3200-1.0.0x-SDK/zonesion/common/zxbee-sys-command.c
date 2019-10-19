/*********************************************************************************************
* 文件：zxbee-sys-command.c
* 作者：xuzhy 2018.1.12
* 说明：lte 模块公共zxbee指令处理模块
*       
*       
*       
* 修改：fuyou 增加透传驱动部分
* 注释：
*********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zxbee.h"
#include "at.h"
#include "sensor.h"

/*********************************************************************************************
* 名称：ZXBeeSysCommandProc
* 功能：模块公共zxbee指令处理
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int ZXBeeSysCommandProc(char* ptag, char* pval)
{
  int ret = -1;
  if (memcmp(ptag, "ECHO", 4) == 0) {
    ZXBeeAdd(ptag, pval);
    return 1;
  }
  if (memcmp(ptag, "TYPE", 4) == 0) {
    if (pval[0] == '?') {
      char buf[16];
#ifndef CC3200_Serial
      sprintf(buf, "%d2%s", NODE_TYPE, NODE_NAME);
      ZXBeeAdd("TYPE", buf);
      return 1;
#else
      sprintf(buf,"AT+TYPE?\r\n");
      at_notify_data(buf,strlen(buf));
#endif
    }
  }
  return ret;
}



