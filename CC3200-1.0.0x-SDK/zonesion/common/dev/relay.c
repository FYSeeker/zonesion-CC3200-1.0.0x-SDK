/*********************************************************************************************
* �ļ���relay.c
* ���ߣ�Lixm 2017.10.17
* ˵�����̵�����������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "relay.h"

/*********************************************************************************************
* ���ƣ�relay_init()
* ���ܣ��̵�����������ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void relay_init(void)
{
  PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);      //ʹ��ʱ��
  PinTypeGPIO(PIN_08,PIN_MODE_0,0);                           //ѡ������ΪGPIOģʽ��gpio17��
  GPIODirModeSet(GPIOA2_BASE, G17_UCPINS, GPIO_DIR_MODE_OUT);   //����GPIO17Ϊ���ģʽ
  
  
  PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);      //ʹ��ʱ��
  PinTypeGPIO(PIN_18,PIN_MODE_0,0);                           //ѡ������ΪGPIOģʽ��gpio28��
  GPIODirModeSet(GPIOA3_BASE, G28_UCPINS, GPIO_DIR_MODE_OUT);   //����GPIO28Ϊ���ģʽ
  
  GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0xFF);
  GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0xFF);                  //��ʼ���Ͽ��̵���
}

signed int relay_on(char cmd)
{
  if(cmd & 0x01){
    GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0x00);
   
  }
  if(cmd & 0x02){
    GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0x00);
    
  }
   return 0;
}


signed int relay_off(char cmd)
{
  if(cmd & 0x01){
    GPIOPinWrite(GPIOA2_BASE, G17_UCPINS, 0xff);
    
  }
  if(cmd & 0x02){
    GPIOPinWrite(GPIOA3_BASE, G28_UCPINS, 0xff);
    
  }
  return 0;
}

void relay_control(char cmd)
{
  relay_on(cmd);
  relay_off((~cmd)&0x03);
}
