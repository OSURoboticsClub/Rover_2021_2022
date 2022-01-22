/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	
	//GPIO Setup
	//ALL GPIO are on port A
	pio_set_output(PIOA,BUILT_IN_LED,LOW,DISABLE,DISABLE);
	pio_set_output(PIOA,MUX_EN,HIGH,DISABLE,DISABLE);
	pio_set_output(PIOA,MUX_A0,HIGH,DISABLE,DISABLE);
	pio_set_output(PIOA,MUX_A1,HIGH,DISABLE,DISABLE);
	pio_set_output(PIOA,MUX_A2,HIGH,DISABLE,DISABLE);
	
	pmc_enable_periph_clk(ID_ADC);
	//ADC Setup
	/* Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 6.4MHz = 10 us
	 */
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, ADC_STARTUP_TIME_4);
	/* Formula:
	 *     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	 *     Tracking Time = (TRACKTIM + 1) / ADCClock
	 *     Settling Time = settling value / ADCClock
	 *
	 *     Transfer Time = (1 * 2 + 3) / 6.4MHz = 781 ns
	 *     Tracking Time = (1 + 1) / 6.4MHz = 312 ns
	 *     Settling Time = 3 / 6.4MHz = 469 ns
	 */
	adc_configure_timing(ADC, TRACKING_TIME, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);
	//adc_set_resolution(ADC,12);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);
	adc_enable_channel(ADC, BOARD_TEMP1);
	adc_enable_channel(ADC, BOARD_TEMP2);
	adc_enable_channel(ADC, BOARD_CELLV);
	adc_enable_channel(ADC, BOARD_SHUNT);
}
