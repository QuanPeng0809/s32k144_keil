
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-04     suozhang      the first version
 *
 */

#include "bsp_vlps.h"

#include "device_registers.h"
#include "system_S32K144.h"
#include "stdbool.h"

#include "SEGGER_RTT.h"


/*******************************************************************************
Function: enter_VLPS
Notes   : VLPS in Sleep-On-Exit mode
        : Should VLPS transition failed, reset the MCU
*******************************************************************************/
void vlps_init( void )
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
	
	SEGGER_RTT_printf( 0, "SMC->PMPROT:0x%x.\r\n", SMC->PMPROT );
	SEGGER_RTT_printf( 0, "S32_SCB->SCR:0x%x.\r\n", S32_SCB->SCR );
	SEGGER_RTT_printf( 0, "PMC->REGSC:0x%x.\r\n", PMC->REGSC );
	SEGGER_RTT_printf( 0, "SMC->PMCTRL:0x%x.\r\n", SMC->PMCTRL );

}