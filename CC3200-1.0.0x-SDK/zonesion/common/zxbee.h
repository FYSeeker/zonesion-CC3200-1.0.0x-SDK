#ifndef __ZXBEE_H__
#define __ZXBEE_H__



typedef unsigned char    uint8 ;
typedef unsigned short   uint16;
typedef unsigned int     uint32;


extern char SENSOR_TYPE[];
void ZXBeeInit(void);
void ZXBeeBegin(void);
void ZXBeeAdd(char* pt, char*pv);
char* ZXBeeEnd(void);
char* ZXBeeDecodePackage(char *buf, int len);
int ZXBeeUserProcess(char *ptag, char *pval);
int ZXBeeSysCommandProc(char* ptag, char* pval);

#endif