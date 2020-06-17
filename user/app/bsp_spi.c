
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-15     suozhang      the first version
 *
 */
 
#include "bsp_spi.h"

#include "device_registers.h"
#include "system_S32K144.h"
#include "stdbool.h"

#include "SEGGER_RTT.h"

void bsp_lpspi0_port_init( void )
{
  PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;  /* Enable clock for PORTB */
  PORTB->PCR[2] |= PORT_PCR_MUX(3);           			/* Port B2: MUX = ALT3, LPSPI1_SCK */
  PORTB->PCR[3] |= PORT_PCR_MUX(3);           			/* Port B3: MUX = ALT3, LPSPI1_SIN */
  PORTB->PCR[4] |= PORT_PCR_MUX(3);           			/* Port B4: MUX = ALT3, LPSPI1_SOUT */
  PORTB->PCR[5] |= PORT_PCR_MUX(3);           			/* Port B5: MUX = ALT3, LPSPI1_PCS3 */
}

void bsp_lpspi1_port_init( void )
{
  PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;  /* Enable clock for PORTB */
  PORTB->PCR[14] |= PORT_PCR_MUX(3);           			/* Port B14: MUX = ALT3, LPSPI1_SCK */
  PORTB->PCR[15] |= PORT_PCR_MUX(3);           			/* Port B15: MUX = ALT3, LPSPI1_SIN */
  PORTB->PCR[16] |= PORT_PCR_MUX(3);           			/* Port B16: MUX = ALT3, LPSPI1_SOUT */
  PORTB->PCR[17] |= PORT_PCR_MUX(3);           			/* Port B17: MUX = ALT3, LPSPI1_PCS3 */
}


uint8_t init_lpspi0()
{
	ENABLE_GPIO_CLOCK(PORTB);    //Enable GPIO clock at gpio_init() function in gpio.c
	PCC->PCCn[PCC_LPSPI0_INDEX] = PCC_PCCn_PCS(1);    //select spi clock source
	PCC->PCCn[PCC_LPSPI0_INDEX] |= PCC_PCCn_CGC_MASK;    //enable SPI clock
	PORTB->PCR[2] |=  0x00000300;    //port configure as chip-specific
	PORTB->PCR[3] |=  0x00000300;
	PORTB->PCR[4]  |=  0x00000300;
	PORTB->PCR[5]  |=  0x00000300;    //use as GPIO

	LPSPI0->CFGR1 |= LPSPI_CFGR1_MASTER_MASK;    //select as master mode

	LPSPI0->CFGR1 &= ~(	 LPSPI_CFGR1_PINCFG_MASK  |     //SIN is used for input data and SOUT for output data.
											 LPSPI_CFGR1_OUTCFG_MASK  |     //Output data retains last value when chip select is negated.
											 LPSPI_CFGR1_PCSCFG_MASK  |
											 LPSPI_CFGR1_MATCFG_MASK  |     //Match disabled.
											 LPSPI_CFGR1_NOSTALL_MASK |     //transfers will stall when transmit FIFO is empty or receive FIFO is full.
											 LPSPI_CFGR1_SAMPLE_MASK );     //Input data sampled on SCK edge.
//        LPSPI0->CFGR1 |= LPSPI_CFGR1_PCSPOL(0B0010);    //Configures the polarity of each Peripheral Chip Select(PCS) pin

//        LPSPI0->CCR = 0x001F0000;    //1F just for test ,true PCS to SCK delay to be define
    LPSPI0->FCR   &= ~(LPSPI_FCR_RXWATER_MASK |
                                 LPSPI_FCR_TXWATER_MASK);
    LPSPI0->FCR   |= LPSPI_FCR_RXWATER(0U) | LPSPI_FCR_TXWATER(1U);
    if(LPSPI0->SR & LPSPI_SR_MBF_MASK)    //make sure SPI is idle
            return 0;
    if(LPSPI0->CR & LPSPI_CR_MEN_MASK)    //make sure SPI is not enable
               return 0;

           LPSPI0->TCR = 0x59000007;    //TCR can writer only once.
                                 //PCS1

        LPSPI0->CR |= LPSPI_CR_MEN_MASK;    //enable SPI mode
        return 1;
}

void bsp_lpspi1_init( void )
{

	SEGGER_RTT_printf( 0, "bsp_lpspi1_init.\r\n" );
  
	// select spi clock source
	/* SPI ʱ��Դ��4��: SPLLDIV2_CLK,FIRCDIV2_CLK,SIRCDIV2_CLK,SOSCDIV2_CLK */
	PCC->PCCn[PCC_LPSPI1_INDEX]  = PCC_PCCn_PCS(1);
	
	// enable SPI clock
	// ʹ������Ľӿ�ʱ�ӣ�������� LPSPI1 ģ��
	PCC->PCCn[PCC_LPSPI1_INDEX] |= PCC_PCCn_CGC_MASK;
  
	// select as master mode
	// ����Ϊ ���� ģʽ
	LPSPI1->CFGR1 |= LPSPI_CFGR1_MASTER(1);
  
	// set Configuration Pin mode
	// SIN is used for input data and SOUT for output data.
	// ������ģʽ ���� ʵ�� SI �� SOUT ���� ������ PCB ���� ���� �øù������
	LPSPI1->CFGR1 &= ~(LPSPI_CFGR1_PINCFG_MASK); // �Ĵ���λ ���� ����
	LPSPI1->CFGR1 |= LPSPI_CFGR1_PINCFG(0);
  
	// set Output Config
	// Output data retains last value when chip select is negated.
	// ���� LPSPI_PCS �ߵ� ������ ����������� �������һ�� ����״̬ ���� ��̬
	// �ӻ�ģʽ ��
	LPSPI1->CFGR1 &= ~(LPSPI_CFGR1_OUTCFG_MASK); // �Ĵ���λ ���� ����
	LPSPI1->CFGR1 |= LPSPI_CFGR1_OUTCFG(0);
	
	
	
	LPSPI1->CFGR1 &= ~(	 LPSPI_CFGR1_PINCFG_MASK  |     // SIN is used for input data and SOUT for output data.
											 LPSPI_CFGR1_OUTCFG_MASK  |     // Output data retains last value when chip select is negated.
											 LPSPI_CFGR1_PCSCFG_MASK  |			// 
											 LPSPI_CFGR1_MATCFG_MASK  |     // Match disabled.
											 LPSPI_CFGR1_NOSTALL_MASK |     // transfers will stall when transmit FIFO is empty or receive FIFO is full.
											 LPSPI_CFGR1_SAMPLE_MASK );     // Input data sampled on SCK edge.
	
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











