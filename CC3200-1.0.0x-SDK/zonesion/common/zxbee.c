#include <string.h>
#include <stdio.h>

#include "zxbee.h"
#include "zxbee-inf.h"

#define  ZXBEE_BUFFER_SIZE      1024


static char     msgTx[ZXBEE_BUFFER_SIZE];
/*********************************************************************************************
* ���ƣ�ZXBeeBegin
* ���ܣ�ZXBeeָ��ϳɿ�ʼ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeBegin(void)
{
  msgTx[0] = '{';
  msgTx[1] = '\0';
}
/*********************************************************************************************
* ���ƣ�ZXBeeAdd
* ���ܣ����һ�����ݵ�zxbeeָ��
* ������tag����������ʶ
        val��������ֵ
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�ZXBeeEnd
* ���ܣ�zxbeeָ����ӽ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
* ���ƣ�ZXBeeDecodePackage
* ���ܣ�zxbeeָ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
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
