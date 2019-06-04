
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-04     suozhang      the first version
 *
 */
 
#include "freertos_tickless.h"

#include "device_registers.h"
#include "system_S32K144.h"
#include "stdbool.h"

#include "SEGGER_RTT.h"

/* tickless ģʽ����, ��0���ر�ticklessģʽ */

#define tickless_MODE_ON													0x00000000UL  /* tickless ģʽǿ�ƴ� */

#define tickless_BIT_EN														0x01000000UL  /* tickless �ܿ��� ��־λ */
#define tickless_BIT_USEING_DEVICE_UART1					0x02000000UL  /* ����1��������ʹ���� ��־λ */
#define tickless_BIT_USEING_DEVICE_CAN1						0x04000000UL  /* CAN1��������ʹ���� ��־λ */


volatile uint32_t tickless_mode_now = tickless_MODE_ON; /* ��ʼ�� tickless ģʽ */

void tickless_bit_on( uint32_t tickless_bit )
{
	vTaskSuspendAll(); /* �б�Ҫ����Ϊ��λ���ƣ��о�û��Ҫ */

		tickless_mode_now |= tickless_bit;
	
	xTaskResumeAll();

}

void tickless_bit_off( uint32_t tickless_bit )
{

	vTaskSuspendAll(); /* �б�Ҫ����Ϊ��λ���ƣ��о�û��Ҫ */

		tickless_mode_now &= ~tickless_bit;
	
	xTaskResumeAll();

}

/*******************************************************************************
Function: enter_VLPS
Notes   : VLPS in Sleep-On-Exit mode
        : Should VLPS transition failed, reset the MCU
*******************************************************************************/
void vlps_init(void)
{

	// Allow Very-Low-Power Modes
	// [5] AVLP = 1 VLPS allowed
	// ϵͳ��λ������Ĵ���ֻ����д��һ��
	// The PMPROT register can be written only once after any system reset.
	SMC->PMPROT |= SMC_PMPROT_AVLP(1);

	// ϵͳ���ƼĴ��� �� Cortex m4 Generic User Guide.pdf ������ؼĴ�������
	// S32_SCB_SCR_SLEEPDEEP = 1: ��ʾѡ�����˯��ģʽ
	S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP(1);

	// Bias Enable Bit
	// This bit must be set to 1 when using VLP* modes.
	PMC->REGSC |= PMC_REGSC_BIASEN_MASK;
	
	// Stop Mode Control
	// [2-0] STOPM = Very-Low-Power Stop (VLPS)
	SMC->PMCTRL |= SMC_PMCTRL_STOPM(2);
	
	SEGGER_RTT_printf( 0, "SMC->PMPROT:%x.\r\n", SMC->PMPROT );
	SEGGER_RTT_printf( 0, "S32_SCB->SCR:%x.\r\n", S32_SCB->SCR );
	SEGGER_RTT_printf( 0, "PMC->REGSC:%x.\r\n", PMC->REGSC );
	SEGGER_RTT_printf( 0, "SMC->PMCTRL:%x.\r\n", SMC->PMCTRL );

}

/*******************************************************************************
Function: LPTMR0_IRQHandler
*******************************************************************************/
static volatile uint32_t lptmr_csr = 0; // ������ǰ����״̬��־λ�������ж�ʧ�ܣ���־λ�ᱻ�������֪Ϊɶ
void LPTMR0_IRQHandler(void)
{

	// ʹ�� LPTMR ����Ľӿ�ʱ�ӣ�������� LPTMR ģ��
	PCC->PCCn[PCC_LPTMR0_INDEX] = PCC_PCCn_CGC_MASK; // Enable clock to LPTMR module - BUS_CLK

	SEGGER_RTT_printf( 0, "\r\nLPTMR0_IRQHandler tick:%u.\r\n", xTaskGetTickCount() );
	
	// ��ǰ����״̬��־λ�����������ж�ʧ�ܣ���־λ�ᱻ�������֪Ϊɶ
	lptmr_csr = LPTMR0->CSR;
	
		SEGGER_RTT_printf( 0, "LPTMR0->CNR:%u.\r\n", LPTMR0->CNR );
		SEGGER_RTT_printf( 0, "LPTMR0->CSR:%X.\r\n", LPTMR0->CSR );

	// ��ʱ���ж�ʧ��
	LPTMR0->CSR &= ~LPTMR_CSR_TIE(0);

}

/*******************************************************************************
Function: init_lptmr_tick_interrupt
Notes   : 32kHz RTC clock derived from 128kHz internal LPO
        : Time counter mode, interrupt enabled
				: 1 tick == 1ms
*******************************************************************************/
void init_lptmr_tick_interrupt( TickType_t xExpectedIdleTime )
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
    S32_NVIC->ICPR[1] = (1 << (58 % 32)); //58: s32k144.h �ļ��� LPTMR0_IRQn ��ֵ
    S32_NVIC->ISER[1] = (1 << (58 % 32));
    S32_NVIC->IP[58]  = 15;  // Priority level 15

    // [0] TEN  = 1 LPTMR ������ʱ��
	  LPTMR0->CSR |= LPTMR_CSR_TEN(1);

}



void application_sleep_enter_before( TickType_t xExpectedIdleTime )
{

	SEGGER_RTT_printf( 0, "application_sleep_enter_before:\r\n" );
	
}

void application_sleep_enter_later( TickType_t xExpectedIdleTime )
{

	SEGGER_RTT_printf( 0, "application_sleep_enter_later:\r\n" );

	
}

uint32_t get_lpmrt_counter( void )
{
	return LPTMR0->CNR;
}


#ifndef configSYSTICK_CLOCK_HZ
	#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
	/* Ensure the SysTick is clocked at the same frequency as the core. */
	#define portNVIC_SYSTICK_CLK_BIT_SUOZHANG	( 1UL << 2UL )
#else
	/* The way the SysTick is clocked is not modified in case it is not the same
	as the core. */
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#endif

#define portNVIC_SYSTICK_INT_BIT_SUOZHANG			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT_SUOZHANG			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT_SUOZHANG		( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT_SUOZHANG 			( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT_SUOZHANG		( 1UL << 25UL )
/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 16 bit resolution of the Low Power Timer.
 */
#if( configUSE_TICKLESS_IDLE == 1 )
	static uint32_t xMaximumPossibleSuppressedTicks = 60000;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The number of SysTick increments that make up one tick period.
 */
#if( configUSE_TICKLESS_IDLE == 1 )
	static uint32_t ulTimerCountsForOneTick = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ );
#endif /* configUSE_TICKLESS_IDLE */

#define portNVIC_SYSTICK_CTRL_REG_SUOZHANG					( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG_SUOZHANG					( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG_SUOZHANG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSPRI2_REG_SUOZHANG								( * ( ( volatile uint32_t * ) 0xe000ed20 ) )

#if( configUSE_TICKLESS_IDLE == 1 )

void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
	uint32_t ulReloadValue, ulCompleteTickPeriods;

	TickType_t xModifiableIdleTime;
	
		/* �ж��Ƿ���� tickless ģʽ */
		if( tickless_mode_now ) 
		{
			// ��ֹ���� tickless ģʽ
			return;
		
		}

		/* Make sure the SysTick reload value does not overflow the counter. */
		if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
		{
			xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
		}

		/* Stop the SysTick momentarily.  The time the SysTick is stopped for
		is accounted for as best it can be, but using the tickless mode will
		inevitably result in some tiny drift of the time maintained by the
		kernel with respect to calendar time. */
		portNVIC_SYSTICK_CTRL_REG_SUOZHANG &= ~portNVIC_SYSTICK_ENABLE_BIT_SUOZHANG;

		/* Calculate the reload value required to wait xExpectedIdleTime
		tick periods.  -1 is used because this code will execute part way
		through one of the tick periods. */
		ulReloadValue = xExpectedIdleTime - 1UL;

		/* Enter a critical section but don't use the taskENTER_CRITICAL()
		method as that will mask interrupts that should exit sleep mode. */
		__disable_irq();
		__dsb( portSY_FULL_READ_WRITE );
		__isb( portSY_FULL_READ_WRITE );

		/* If a context switch is pending or a task is waiting for the scheduler
		to be unsuspended then abandon the low power entry. */
		if( eTaskConfirmSleepModeStatus() == eAbortSleep )
		{
			/* Restart from whatever is left in the count register to complete
			this tick period. */
			portNVIC_SYSTICK_LOAD_REG_SUOZHANG = portNVIC_SYSTICK_CURRENT_VALUE_REG_SUOZHANG;

			/* Restart SysTick. */
			portNVIC_SYSTICK_CTRL_REG_SUOZHANG |= portNVIC_SYSTICK_ENABLE_BIT_SUOZHANG;

			/* Reset the reload register to the value required for normal tick
			periods. */
			portNVIC_SYSTICK_LOAD_REG_SUOZHANG = ulTimerCountsForOneTick - 1UL;

			/* Re-enable interrupts - see comments above __disable_irq() call
			above. */
			__enable_irq();
		}
		else
		{
			/* Set LPTMR the new reload value. */
			init_lptmr_tick_interrupt( ulReloadValue );

			/* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
			set its parameter to 0 to indicate that its implementation contains
			its own wait for interrupt or wait for event instruction, and so wfi
			should not be executed again.  However, the original expected idle
			time variable must remain unmodified, so a copy is taken. */
			xModifiableIdleTime = xExpectedIdleTime;
			configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
			if( xModifiableIdleTime > 0 )
			{
				__dsb( portSY_FULL_READ_WRITE );
				STANDBY(); /* suozhang, s32k144 enter VLPS mode */
				__isb( portSY_FULL_READ_WRITE );
			}
			configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

			/* Re-enable interrupts to allow the interrupt that brought the MCU
			out of sleep mode to execute immediately.  see comments above
			__disable_interrupt() call above. */
			__enable_irq();
			__dsb( portSY_FULL_READ_WRITE );
			__isb( portSY_FULL_READ_WRITE );

			/* Disable interrupts again because the clock is about to be stopped
			and interrupts that execute while the clock is stopped will increase
			any slippage between the time maintained by the RTOS and calendar
			time. */
			__disable_irq();
			__dsb( portSY_FULL_READ_WRITE );
			__isb( portSY_FULL_READ_WRITE );

			if(lptmr_csr & LPTMR_CSR_TCF_MASK)
			{/* �ǵ͹��Ķ�ʱ�����ѵ�CPU */

				SEGGER_RTT_printf( 0, "lptmr wake up.\r\n" );
				
				/* As the pending tick will be processed as soon as this
				function exits, the tick value maintained by the tick is stepped
				forward by one less than the time spent waiting. */
				ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
				
			}
			else
			{/* �����жϻ��ѵ�CPU */

				SEGGER_RTT_printf( 0, "not lptmr wake up.\r\n" );

				/* ��ȡ LPTMR Counter Register������ system tick ���� */
				/* ����ֱ�Ӷ�ȡ�Ĵ��� �Ῠ������֪Ϊɶ */
				ulCompleteTickPeriods = get_lpmrt_counter() ;
				
				// ����������, ��ʱ��������������������ʱ�䣬FreeRTOS ���ж��Կ��������������
				if( ulCompleteTickPeriods >= xExpectedIdleTime )
				{
					SEGGER_RTT_printf( 0, "ulCompleteTickPeriods :%d.\r\n", ulCompleteTickPeriods );
					
					ulCompleteTickPeriods = xExpectedIdleTime;
				}


			}

			/* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
			again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
			value. */
			portNVIC_SYSTICK_CURRENT_VALUE_REG_SUOZHANG = 0UL;
			portNVIC_SYSTICK_CTRL_REG_SUOZHANG |= portNVIC_SYSTICK_ENABLE_BIT_SUOZHANG;
			vTaskStepTick( ulCompleteTickPeriods );
			portNVIC_SYSTICK_LOAD_REG_SUOZHANG = ulTimerCountsForOneTick - 1UL;

			/* Exit with interrpts enabled. */
			__enable_irq();
		}
	}
	
#endif /* configUSE_TICKLESS_IDLE */


















