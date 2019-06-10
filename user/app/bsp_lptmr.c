
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-04     suozhang      the first version
 *
 */
 
#include "bsp_lptmr.h"

#include "SEGGER_RTT.h"

// ������ǰ����CSR�Ĵ����ж�״̬��־λ���������жϺ����н����ж�ʧ�ܣ���־λ�ᱻ�������֪Ϊɶ
// Ŀ�����ж� ��ʱ�ж��Ƿ����
static volatile uint32_t lptmr_csr = 0;

/*******************************************************************************
Function: LPTMR0_IRQHandler
*******************************************************************************/
void LPTMR0_IRQHandler(void)
{

	// ʹ�� LPTMR ����Ľӿ�ʱ�ӣ�������� LPTMR ģ��
	PCC->PCCn[PCC_LPTMR0_INDEX] = PCC_PCCn_CGC_MASK; // Enable clock to LPTMR module - BUS_CLK

	// ��ǰ����״̬��־λ�����������ж�ʧ�ܣ���־λ�ᱻ�������֪Ϊɶ
	lptmr_csr = LPTMR0->CSR;
	
		SEGGER_RTT_printf( 0, "LPTMR0->CNR:%u.\r\n", LPTMR0->CNR );
		SEGGER_RTT_printf( 0, "LPTMR0->CSR:%X.\r\n", LPTMR0->CSR );

	// ��ʱ���ж�ʧ��
	LPTMR0->CSR &= ~LPTMR_CSR_TIE(0);

}

/*******************************************************************************
Function: init_lptmr_tick
Notes   : 32kHz RTC clock derived from 128kHz internal LPO
        : Time counter mode, interrupt enabled
				: 1 tick == 1ms
*******************************************************************************/
void init_lptmr_tick( uint32_t xExpectedIdleTime )
{
		SEGGER_RTT_printf( 0, "init_lptmr_tick_interrupt xExpectedIdleTime tick:%u.\r\n", xExpectedIdleTime );
	
    // Peripheral Clock Control for LPTMR
		// ʹ�� LPTMR ����Ľӿ�ʱ�ӣ�������� LPTMR ģ��
    PCC->PCCn[PCC_LPTMR0_INDEX] = PCC_PCCn_CGC_MASK; // Enable clock to LPTMR module - BUS_CLK

		LPTMR0->CNR |= 0XFFFF0000; 
	
		lptmr_csr = 0; //��0
	
		// [6] TIE  = 1 enable time interrupt
    // [2] TFC  = 1 CNR is reset on overflow.
    // [1] TMS  = 0 ʱ�������ģʽ
    // [0] TEN  = 0 LPTMR ��ʱ���ر�
    LPTMR0->CSR = LPTMR_CSR_TFC(1) | LPTMR_CSR_TIE(1);
	
		/* LPTMR ʱ��Դ��4��: 1: SIRCDIV2_CLK, 2: LPO1K_CLK, 3: RTC_CLK, 4: BUS_CLK */
		/* FreeRTOS �� 1ms һ�� tick, �����Ҫ���� LPTMR ���� ��λ�� 1ms ���ж� */
		/* S32K144 VLPSģʽ�£�ֻ��ѡ�� LPO1K_CLK, ���� RTC_CLK  */
		/* ����ѡ�� RTC_CLK,��Ҫ����RTC_CLKʱ��Դ�����Ĵ��� SIM->LPOCLKS[3-2] LPOCLKSEL,ѡ�� LPO32K_CLK */
		/* ������� LPO32k_CLK �����Ҫʹ�� */
	
		/* ���󾯸棺���������� RTC ��ʱ��Դ����Ӱ��RTC ��ʱ�ӣ����߿��ܻ��ͻ����ע�� */
		/* ���󾯸棺LPO32k_CLK �� LPO1k_CLK ����ͬʱʹ�ܣ������ѡ�� 1K�������ע�� */
	
		/* �ڶ���ʱ�ӷ�����Ԥ��Ƶ����������·����Ҳ����˵ ʱ��Դֱ�ӵ� �͹��Ķ�ʱ�������Ĵ�������˿���ѡ�� LPO1K_CLK */
	
    // [5-4] RTCCLKSEL = 1 choose LPO32k_CLK
    // [1] LPO32KCLKEN = 1 Enable LPO32k_CLK output
    // [0] LPO1KCLKEN =  0 Disable LPO1k_CLK  output
		SIM->LPOCLKS = SIM_LPOCLKS_RTCCLKSEL(1) | SIM_LPOCLKS_LPO32KCLKEN(1) | SIM_LPOCLKS_LPO1KCLKEN(0);

    // LPO32K_CLK ʱ�� ��Ԥ��Ƶ������32��Ƶ��ÿ16�������أ������Ĵ���+1
		// ���� �����Ĵ��� ��1 ���� 1ms
		// [6-3] PRESCALE  = 4 ѡ�� 32 ��Ƶ
    // [2] 	 PBYP      = 0 Enable Ԥ��Ƶ��
    // [1-0] PCS       = 2 �����ֲ�2018.9.9 table 27-9 , ����ѡ��RTC_CLK
		LPTMR0->PSR = LPTMR_PSR_PRESCALE(4) | LPTMR_PSR_PBYP(0) | LPTMR_PSR_PCS(2);
		
    // �趨�͹��Ķ�ʱ���ȽϼĴ���
    LPTMR0->CMR = xExpectedIdleTime; //��ʱ xExpectedIdleTime �� tick �� �����ж� 
		
		// LPTMR0_Interrupt
    S32_NVIC->ICPR[1] = (1 << (LPTMR0_IRQn % 32));
    S32_NVIC->ISER[1] = (1 << (LPTMR0_IRQn % 32));
    S32_NVIC->IP[LPTMR0_IRQn]  = 15;  // Priority level 15

    // [0] TEN  = 1 LPTMR ������ʱ��
	  LPTMR0->CSR |= LPTMR_CSR_TEN(1);

}

uint32_t get_lpmrt_counter( void )
{
	return LPTMR0->CNR;
}

// ���ض�ʱ���Ƿ�����ж�
// true: �����ж�
// false: δ�����ж�
bool get_lpmrt_time_interrupt_flag( void )
{
	return (lptmr_csr & LPTMR_CSR_TCF_MASK);
}





