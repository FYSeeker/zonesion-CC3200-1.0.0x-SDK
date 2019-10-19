/*********************************************************************************************
* �ļ���zxbee-sys-command.c
* ���ߣ�xuzhy 2018.1.12
* ˵����lte ģ�鹫��zxbeeָ���ģ��
*       
*       
*       
* �޸ģ�fuyou ����͸����������
* ע�ͣ�
*********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zxbee.h"
#include "at.h"
#include "sensor.h"

/*********************************************************************************************
* ���ƣ�ZXBeeSysCommandProc
* ���ܣ�ģ�鹫��zxbeeָ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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



