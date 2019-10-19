/*********************************************************************************************
* 文件：stadiometry.c
* 作者：Lixm 2017.10.17
* 说明：红外测距驱动代码
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "stadiometry.h"
#include "adc.h"
/*********************************************************************************************
* 名称：stadiometry_init()
* 功能：红外测距传感器初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void stadiometry_init(void)
{
  PinTypeADC(PIN_59,PIN_MODE_255);
  ADCChannelEnable(ADC_BASE, ADC_CH_2);//使能ADC通道3
  //ADCTimerConfig(ADC_BASE, 2^17);       //配置ADC内部定时器
  //ADCTimerEnable(ADC_BASE);             //使能内部定时器   
  ADCEnable(ADC_BASE);                  //ADC使能
}

/*********************************************************************************************
* 名称：float get_stadiometry_data(void)
* 功能：获取红外测距传感器状态
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
float get_stadiometry_data(void)
{ 
 
  float dis = 0, v = 0;
  static unsigned int  values[1024];
  
  
  while(ADCFIFOLvlGet(ADC_BASE, ADC_CH_2)){
    ADCFIFORead(ADC_BASE, ADC_CH_2); //清adc缓存
  }
  for (int i=0; i<1024; i++) {
    while(!ADCFIFOLvlGet(ADC_BASE, ADC_CH_2));
    values[i] = ADCFIFORead(ADC_BASE, ADC_CH_2);
    //v += ((value >> 2 ) & 0x0FFF)*1.47/4096;  //计算电压
  }
  for (int i=1023; i>0; i--){
    for (int j=0; j<i; j++) {
      if (values[j] > values[j+1]) {
        int t = values[j];
        values[j] = values[j+1];
        values[j+1] = t;
      }
    }
  }
  int j = 0;
  for (int i=100; i<1024-100; i++) {
    j += 1;
    v += ((values[i] >> 2 ) & 0x0FFF)*1.47/4096;  //计算电压
  }
  
  v = v / j;

  if (v>=0.4f && v<1.4f) {
    dis = 235 + (400-235)/(0.7f-1.15f)*(v-1.15f);
  }
  
  //Report("v = %.2f, d %.0fmm\r\n", v, dis);
  return dis/10.0f;
  
}