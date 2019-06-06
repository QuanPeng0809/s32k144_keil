
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-04     suozhang      the first version
 *
 */


#ifndef FREERTOS_TICKLESS_H
#define FREERTOS_TICKLESS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bsp_lptmr.h"


// �͹��Ķ�ʱ����ʼ�� xExpectedIdleTime ��ʱ���ѣ��ӿ��ض��򣬷�����ֲ
#define port_low_power_time_tick( xExpectedIdleTime )   init_lptmr_tick( xExpectedIdleTime )

// ��ȡ�͹��Ķ�ʱ���Ƿ����жϣ��ӿ��ض��򣬷�����ֲ
#define port_get_low_power_time_interrupt_flag( void )  get_lpmrt_time_interrupt_flag( void )

// ��ȡ�͹��Ķ�ʱ������ֵ���ӿ��ض��򣬷�����ֲ
#define port_get_low_power_time_counter( void ) 				get_lpmrt_counter( void )

/* tickless ģʽ����, ��0���ر�ticklessģʽ */

#define tickless_MODE_ON													0x00000000UL  /* tickless ģʽǿ�ƴ� */

#define tickless_BIT_EN														0x01000000UL  /* tickless �ܿ��� ��־λ */
#define tickless_BIT_USEING_DEVICE_UART1					0x02000000UL  /* ����1��������ʹ���� ��־λ */
#define tickless_BIT_USEING_DEVICE_CAN1						0x04000000UL  /* CAN1��������ʹ���� ��־λ */

#endif /* FREERTOS_TICKLESS_H */

