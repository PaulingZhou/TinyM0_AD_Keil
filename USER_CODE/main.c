/****************************************Copyright (c)****************************************************
**                            Guangzhou ZLGMCU Development Co., LTD
**
**                                 http://www.zlgmcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           main.c
** Last modified Date:  2010-02-04
** Last Version:        V1.0
** Descriptions:        The main() function example template
**
**--------------------------------------------------------------------------------------------------------
** Created by:          Li Baihua
** Created date:        2010-09-10
** Version:             V1.00
** Descriptions:        整理模板，添加用户应用程序
**
**--------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Descriptions:        
**
**--------------------------------------------------------------------------------------------------------
** Modified by:        
** Modified date:      
** Version:            
** Descriptions:       
**
** Rechecked by:
*********************************************************************************************************/
#include "LPC11xx.h"                                                    /* LPC11xx外设寄存器            */
#include <stdio.h>

/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define UART_BPS    115200                                              /*  串口通信波特率              */
char    GcRcvBuf[20];                                                   /* AD采集到的数据               */

/*********************************************************************************************************
** Function name:       myDelay
** Descriptions:        软件延时
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void myDelay(uint32_t ulTime)
{
   uint32_t i;

   while (ulTime--) {
       for (i = 0; i < 5000; i++);
   }
}

/*********************************************************************************************************
** Function name:       ADCInit
** Descriptions:        ADC初始化
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void ADCInit( void )
{
    LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 16);
    LPC_IOCON->R_PIO0_11 &= ~0xBF;                                      /* 配置PIO0_11为模拟输入模式    */
    LPC_IOCON->R_PIO0_11 |=  0x02;                                      /* PIO0_11模拟输入通道0         */

    LPC_SYSCON->PDRUNCFG &= ~(0x01 << 4);                               /* ADC模块上电                  */
    LPC_SYSCON->SYSAHBCLKCTRL |=  (0x01 << 13);                         /* 使能ADC模块时钟              */


    LPC_ADC->CR = ( 0x01 << 0 ) |                                       /* SEL=1,选择ADC0               */
                  (( SystemFrequency / 1000000 - 1 ) << 8 ) |           /* 转换时钟1MHz                 */
                  ( 1 << 16 ) |                                         /* BURST=1,使用Burst模式        */
                  ( 0 << 17 ) |                                         /* 使用11 clocks转换            */
                  ( 0 << 24 ) |                                         /* ADC转换停止                  */
                  ( 0 << 27 );                                          /* 直接启动ADC转换，此位无效    */
}

/*********************************************************************************************************
** Function name:       uartInit
** Descriptions:        串口初始化，设置为8位数据位，1位停止位，无奇偶校验，波特率为115200
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void uartInit (void)
{
    uint16_t usFdiv;

    LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 16);
    LPC_IOCON->PIO1_6  &= ~0x07;
    LPC_IOCON->PIO1_6  |= (1<<0);                                       /* 配置P1.6为RXD                */
    LPC_IOCON->PIO1_7  &= ~0x07;
    LPC_IOCON->PIO1_7  |= (1<<0);                                       /* 配置P1.7为TXD                */
    
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);                               /* 打开UART功能部件时钟         */
    LPC_SYSCON->UARTCLKDIV     = 0x01;                                  /* UART时钟分频                 */

    LPC_UART->LCR  = 0x83;                                              /* 允许设置波特率               */
    usFdiv = (SystemFrequency/LPC_SYSCON->UARTCLKDIV/16)/UART_BPS;      /* 设置波特率                   */
    LPC_UART->DLM  = usFdiv / 256;
    LPC_UART->DLL  = usFdiv % 256; 
    LPC_UART->LCR  = 0x03;                                              /* 锁定波特率                   */
    LPC_UART->FCR  = 0x07;
}

/*********************************************************************************************************
** Function name:       uartSendByte
** Descriptions:        向串口发送子节数据，并等待数据发送完成，使用查询方式
** input parameters:    ucDat:   要发送的数据
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void uartSendByte (uint8_t ucDat)
{
    LPC_UART->THR = ucDat;                                              /*  写入数据                    */
    while ((LPC_UART->LSR & 0x40) == 0);                                /*  等待数据发送完毕            */
}

/*********************************************************************************************************
** Function name:       uartSendStr
** Descriptions:        向串口发送字符串
** input parameters:    puiStr:   要发送的字符串指针
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void uartSendStr (char *pucStr)
{
    while (1){
        if (*pucStr == '\0') break;                                     /*  遇到结束符，退出            */
        uartSendByte (*pucStr++);
    }
}

/*********************************************************************************************************
** Function name:       main
** Descriptions:        主函数(函数入口)
**                      串口参数：UART波特率115200、8个数据位、1个停止位、无奇偶校验位；
**                      跳线连接：分别将P0.11与模拟转换输入VIN、P1.7与TXD、P1.6与RXD短接；
**                      操作方法：打开串口调试助手，运行程序；
**                      现    象：串口调试助手显示下位机返回的采样数据。
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
int main(void)
{
    int32_t  i;
    int32_t  ulADCData; 
    int32_t  ulADCBuf;

    SystemInit();                                                       /* 系统初始化                   */

    uartInit();                                                         /* 串口初始化                   */
    ADCInit();                                                          /* ADC模块初始化                */
    while (1) {
        ulADCData = 0;
        for(i = 0; i < 10; i++) {
            while((LPC_ADC->DR[0] & 0x80000000) == 0);                  /* 读取AD0DR0的Done             */
            ulADCBuf = LPC_ADC->DR[0];                                  /* 读取结果寄存器               */
            ulADCBuf = (ulADCBuf >> 6) & 0x3ff;
            ulADCData += ulADCBuf;
        }
        ulADCData = ulADCData / 10;                                     /* 采样10次进行虑波处理         */
        ulADCData = (ulADCData * 3300) / 1024;
        sprintf(GcRcvBuf,"VIN0 = %4d mv\r\n",ulADCData);                /* 将数据发送到串口进行显示     */
        uartSendStr(GcRcvBuf);                                          /* 将数据发送到串口显示         */
        myDelay(50);
    }
}

/*********************************************************************************************************
  End Of File
*********************************************************************************************************/
