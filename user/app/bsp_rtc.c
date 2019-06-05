
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-04     suozhang      the first version
 *
 */
 
#include "bsp_rtc.h"

#include "device_registers.h"
#include "system_S32K144.h"
#include "stdbool.h"

#include "SEGGER_RTT.h"

/*******************************************************************************
Function: RTC_IRQHandler
*******************************************************************************/
void RTC_IRQHandler(void)
{

	RTC->TAR += 3; // Writing to the TAR clears the SR[TAF] Time Alarm Flag.
	
	// Next alarm in 3s
}

/*******************************************************************************
Function: RTC_Seconds_IRQHandler
*******************************************************************************/
void RTC_Seconds_IRQHandler(void)
{

}

/*******************************************************************************
Function: init_rtc
Notes   : ��ʼ�� RTC ��utc_time: ��ǰ UTC ʱ��
				: 32kHz RTC clock derived from 128kHz internal LPO
*******************************************************************************/
void init_rtc(time_t utc_time)
{

    // Power-on cycle the MCU before writing to this register
	
		/* RTC ʱ��Դ��2��: 1: RTC_CLK, 2: LPO1K_CLK */
		/* ����ѡ�� LPO1K_CLK */
		/* ������� LPO1k_CLK �����Ҫʹ�� */

		/* ���󾯸棺SIM->LPOCLKS �Ĵ���ֻ�����ϵ��д��һ�Σ������λ��Ҳ����д�� */

    // [5-4] RTCCLKSEL = 1 choose LPO32k_CLK
    // [1] LPO32KCLKEN = 1 Enable LPO32k_CLK output
    // [0] LPO1KCLKEN =  1 Enable LPO1k_CLK  output
		SIM->LPOCLKS = SIM_LPOCLKS_RTCCLKSEL(1) | SIM_LPOCLKS_LPO32KCLKEN(1) | SIM_LPOCLKS_LPO1KCLKEN(1);

		SEGGER_RTT_printf( 0, "init_rtc SIM->LPOCLKS:0x%x.\r\n", SIM->LPOCLKS );

    // Peripheral Clock Control for RTC
		// ʹ�� RTC ����Ľӿ�ʱ�ӣ�������� RTC ģ��
    PCC->PCCn[PCC_RTC_INDEX] = PCC_PCCn_CGC_MASK; // Enable clock to RTC module - BUS_CLK
	
	  RTC->SR = RTC_SR_TCE(0); // ʧ���������

    RTC->IER = RTC_IER_TAIE(0);
    // [18-16] TSIC = 0x000 1 Hz second interrupt�����ó�1Sһ���ж�
    // [4] TSIE = 0 ���жϹرգ�Time Seconds Interrupt Disable
    // [2] TAIE = 0 �����жϹرգ� Time alarm flag does generate an interrupt
    // [1] TOIE = 0 ����������жϹرգ�Time overflow flag does not generate an interrupt

		SEGGER_RTT_printf( 0, "init_rtc RTC->IER:0x%x.\r\n", RTC->IER );
		
    // Writing to RTC_TSR when the time counter is disabled will clear Time Invalid Flag
		// ʱ����Ĵ��� ��ʼ��
    RTC->TSR = utc_time;

		SEGGER_RTT_printf( 0, "init_rtc RTC->TSR:0x%x.\r\n", RTC->TSR );

    // RTC_TAR Alarm Register
		// ʱ�����ӼĴ�������Ϊ3�������������жϣ���������û������
    RTC->TAR = 3;    // Alarm in 3s

    RTC->CR = RTC_CR_CPE(0) | RTC_CR_LPOS(0);
    // [24] CPE = 0 RTC_CLKOUT is Disabled
    // [9] CLKO = 0 The 32 kHz clock is not output to other peripherals.
    // [7] LPOS = 1 RTC prescaler increments using 1kHz
    // [5] CPS =  0 The prescaler output clock (as configured by TSIC) is output on RTC_CLKOUT

		SEGGER_RTT_printf( 0, "init_rtc RTC->CR:0x%x.\r\n", RTC->CR );
	
		// �������ʹ��
    RTC->SR = RTC_SR_TCE(1);
    // [4] TCE = 1 (Time Counter Enable)

}

/**
 * @ingroup get_rtc_utc_time
 * ��ȡ RTC UTC ʱ��
 * @return ��ǰ RTC UTC ʱ��
 */
time_t get_rtc_utc_time( void )
{
	
	return RTC->TSR;

}












