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
** Descriptions:        ����ģ�壬����û�Ӧ�ó���
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
#include "LPC11xx.h"                                                    /* LPC11xx����Ĵ���            */
#include <stdio.h>

/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define UART_BPS    115200                                              /*  ����ͨ�Ų�����              */
char    GcRcvBuf[20];                                                   /* AD�ɼ���������               */

/*********************************************************************************************************
** Function name:       myDelay
** Descriptions:        �����ʱ
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ADC��ʼ��
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void ADCInit( void )
{
    LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 16);
    LPC_IOCON->R_PIO0_11 &= ~0xBF;                                      /* ����PIO0_11Ϊģ������ģʽ    */
    LPC_IOCON->R_PIO0_11 |=  0x02;                                      /* PIO0_11ģ������ͨ��0         */

    LPC_SYSCON->PDRUNCFG &= ~(0x01 << 4);                               /* ADCģ���ϵ�                  */
    LPC_SYSCON->SYSAHBCLKCTRL |=  (0x01 << 13);                         /* ʹ��ADCģ��ʱ��              */


    LPC_ADC->CR = ( 0x01 << 0 ) |                                       /* SEL=1,ѡ��ADC0               */
                  (( SystemFrequency / 1000000 - 1 ) << 8 ) |           /* ת��ʱ��1MHz                 */
                  ( 1 << 16 ) |                                         /* BURST=1,ʹ��Burstģʽ        */
                  ( 0 << 17 ) |                                         /* ʹ��11 clocksת��            */
                  ( 0 << 24 ) |                                         /* ADCת��ֹͣ                  */
                  ( 0 << 27 );                                          /* ֱ������ADCת������λ��Ч    */
}

/*********************************************************************************************************
** Function name:       uartInit
** Descriptions:        ���ڳ�ʼ��������Ϊ8λ����λ��1λֹͣλ������żУ�飬������Ϊ115200
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void uartInit (void)
{
    uint16_t usFdiv;

    LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 16);
    LPC_IOCON->PIO1_6  &= ~0x07;
    LPC_IOCON->PIO1_6  |= (1<<0);                                       /* ����P1.6ΪRXD                */
    LPC_IOCON->PIO1_7  &= ~0x07;
    LPC_IOCON->PIO1_7  |= (1<<0);                                       /* ����P1.7ΪTXD                */
    
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);                               /* ��UART���ܲ���ʱ��         */
    LPC_SYSCON->UARTCLKDIV     = 0x01;                                  /* UARTʱ�ӷ�Ƶ                 */

    LPC_UART->LCR  = 0x83;                                              /* �������ò�����               */
    usFdiv = (SystemFrequency/LPC_SYSCON->UARTCLKDIV/16)/UART_BPS;      /* ���ò�����                   */
    LPC_UART->DLM  = usFdiv / 256;
    LPC_UART->DLL  = usFdiv % 256; 
    LPC_UART->LCR  = 0x03;                                              /* ����������                   */
    LPC_UART->FCR  = 0x07;
}

/*********************************************************************************************************
** Function name:       uartSendByte
** Descriptions:        �򴮿ڷ����ӽ����ݣ����ȴ����ݷ�����ɣ�ʹ�ò�ѯ��ʽ
** input parameters:    ucDat:   Ҫ���͵�����
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void uartSendByte (uint8_t ucDat)
{
    LPC_UART->THR = ucDat;                                              /*  д������                    */
    while ((LPC_UART->LSR & 0x40) == 0);                                /*  �ȴ����ݷ������            */
}

/*********************************************************************************************************
** Function name:       uartSendStr
** Descriptions:        �򴮿ڷ����ַ���
** input parameters:    puiStr:   Ҫ���͵��ַ���ָ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void uartSendStr (char *pucStr)
{
    while (1){
        if (*pucStr == '\0') break;                                     /*  �������������˳�            */
        uartSendByte (*pucStr++);
    }
}

/*********************************************************************************************************
** Function name:       main
** Descriptions:        ������(�������)
**                      ���ڲ�����UART������115200��8������λ��1��ֹͣλ������żУ��λ��
**                      �������ӣ��ֱ�P0.11��ģ��ת������VIN��P1.7��TXD��P1.6��RXD�̽ӣ�
**                      �����������򿪴��ڵ������֣����г���
**                      ��    �󣺴��ڵ���������ʾ��λ�����صĲ������ݡ�
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
int main(void)
{
    int32_t  i;
    int32_t  ulADCData; 
    int32_t  ulADCBuf;

    SystemInit();                                                       /* ϵͳ��ʼ��                   */

    uartInit();                                                         /* ���ڳ�ʼ��                   */
    ADCInit();                                                          /* ADCģ���ʼ��                */
    while (1) {
        ulADCData = 0;
        for(i = 0; i < 10; i++) {
            while((LPC_ADC->DR[0] & 0x80000000) == 0);                  /* ��ȡAD0DR0��Done             */
            ulADCBuf = LPC_ADC->DR[0];                                  /* ��ȡ����Ĵ���               */
            ulADCBuf = (ulADCBuf >> 6) & 0x3ff;
            ulADCData += ulADCBuf;
        }
        ulADCData = ulADCData / 10;                                     /* ����10�ν����ǲ�����         */
        ulADCData = (ulADCData * 3300) / 1024;
        sprintf(GcRcvBuf,"VIN0 = %4d mv\r\n",ulADCData);                /* �����ݷ��͵����ڽ�����ʾ     */
        uartSendStr(GcRcvBuf);                                          /* �����ݷ��͵�������ʾ         */
        myDelay(50);
    }
}

/*********************************************************************************************************
  End Of File
*********************************************************************************************************/
