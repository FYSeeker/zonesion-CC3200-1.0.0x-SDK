/*********************************************************************************************
* 文件：iic.c
* 作者：zonesion
* 说明：iic驱动程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "iic2.h"
#include <math.h>
#include <stdio.h>

/*********************************************************************************************
* 宏定义
*********************************************************************************************/
#define G10_UCPINS               ( 1 << (10%8))
#define G11_UCPINS               ( 1 << (11%8))
#define SDA_OUT                 GPIODirModeSet(GPIOA1_BASE, G11_UCPINS, GPIO_DIR_MODE_OUT)   //设置GPIO11为输出模式
#define SDA_IN                  GPIODirModeSet(GPIOA1_BASE, G11_UCPINS, GPIO_DIR_MODE_IN)   //设置GPIO11为输出模式

/*********************************************************************************************
* 名称：iic_delay_us()
* 功能：延时函数
* 参数：i -- 延时设置
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
#pragma optimize=none
static void  delay(unsigned int i)
{
  i = i*50;
  while(i--){
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
  }
}

/*********************************************************************************************
* 名称：iic_init()
* 功能：I2C初始化函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic2_init(void)
{
  PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);      //使能时钟
  PinTypeGPIO(PIN_01,PIN_MODE_0,false);                           //选择引脚为GPIO模式（gpio10）
  PinConfigSet(PIN_01,PIN_STRENGTH_4MA,PIN_TYPE_STD_PU);
  GPIODirModeSet(GPIOA1_BASE, G10_UCPINS, GPIO_DIR_MODE_OUT);   //设置GPIO17为输出模式
  
  PinTypeGPIO(PIN_02,PIN_MODE_0,false);                           //选择引脚为GPIO模式（gpio11）
  PinConfigSet(PIN_02,PIN_STRENGTH_4MA,PIN_TYPE_STD_PU);
  GPIODirModeSet(GPIOA1_BASE, G11_UCPINS, GPIO_DIR_MODE_OUT);   //设置GPIO3为输出模式
  
  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0xFF);                                                      //拉高数据线
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);                                                      //拉高时钟线
}

/*********************************************************************************************
* 名称：iic_start()
* 功能：I2C起始信号
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic2_start(void)
{

  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0xFF);                                                      //拉高数据线
  delay(1);  
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);                                                      //拉高时钟线
  delay(1);                                              //延时
  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0x00);                                                      //产生下降沿
  delay(1);                                              //延时
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);                                                      //拉低时钟线
}

/*********************************************************************************************
* 名称：iic_stop()
* 功能：I2C停止信号
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic2_stop(void)
{
  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0x00);                                                        //拉低数据线
  delay(1);  
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);                                                        //拉高时钟线
  delay(1);                                              //延时5us
  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0xFF);                                                         //产生上升沿
}


/*********************************************************************************************
* 名称：iic_write_byte()
* 功能：I2C写一个字节数据，返回ACK或者NACK，从高到低，依次发送
* 参数：data -- 要写的数据
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
unsigned char iic2_write_byte(unsigned char data)
{
  unsigned char i;
                                         //延时2us
  for(i = 0;i < 8;i++){   
   
    if(data & 0x80){                                            //判断数据最高位是否为1
      GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0xFF);
    }
    else{
      GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, 0x00);
    }
    delay(1);                                        
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);	                                                
    delay(1);                                          
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);                                          
                                                
    data <<= 1;                                                 //数组左移一位
  } 

  SDA_IN;
  delay(1);                                              //延时2us
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);                   //拉高时钟线
  if(GPIOPinRead(GPIOA1_BASE, G11_UCPINS) != 0){			//SDA为高，收到NACK
    delay(1); 
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);
    SDA_OUT; 
    return 1;	
  }else{ 				                        //SDA为低，收到ACK
    delay(1); 
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);
    SDA_OUT; 
    return 0;
  }
}

/*********************************************************************************************
* 名称：iic_read_byte()
* 功能：I2C写一个字节数据，返回ACK或者NACK，从高到低，依次发送
* 参数：data -- 要写的数据
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
unsigned char iic2_read_byte(unsigned char ack)
{
  unsigned char i,data = 0;		                                //释放总线

  SDA_IN;
  for(i = 0;i < 8;i++){
    delay(1);	                                        //延时等待信号稳定
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);		         //给出上升沿                        
    data <<= 1;	
    if(GPIOPinRead(GPIOA1_BASE, G11_UCPINS) != 0){ 		//采样获取数据
      data |= 0x01;
    }else{
      data &= 0xfe;
    }
    delay(1);
    GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);	              //给出上升沿
 
  }   

  SDA_OUT;                                                //设置P0_4/P0_5为输出模式
  GPIOPinWrite(GPIOA1_BASE, G11_UCPINS, (ack!=0)?0xff:0x00);	            //应答状态
  delay(1);
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0xFF);                         
  delay(1);          
  GPIOPinWrite(GPIOA1_BASE, G10_UCPINS, 0x00);
  return data;
}


int iic2_read_buf(char addr, char r, char *buf, int len)
{
    int i;
    
    iic2_start();
    if (iic2_write_byte(addr<<1)) {
      iic2_stop();
      return -1;
    }
    if (iic2_write_byte(r)){
     iic2_stop();
      return -1;
    }
    iic2_start();
    if (iic2_write_byte((addr<<1)|1)) {
      iic2_stop();
      return -1;
    }
    for (i=0; i<len-1; i++) {
      buf[i] = iic2_read_byte(0);
    }
    buf[i++] = iic2_read_byte(1);
    iic2_stop();
    return i;
}
int iic2_write_buf(char addr, char r, char *buf, int len)
{
  int i;
  
  iic2_start();
  if (iic2_write_byte(addr<<1)) {
    iic2_stop();
    return -1;
  }

  if (iic2_write_byte(r)) {
    iic2_stop();
    return -1;
  }
 
  for (i=0; i<len; i++) {
    if (iic2_write_byte(buf[i])){
     iic2_stop();
     return -1;
    }
  }
  iic2_stop();
  return i;
}