/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>

#include "conf_clock.h"

#define TMP1		ADC_CHANNEL_4
#define TMP2		ADC_CHANNEL_5
#define CELLV		ADC_CHANNEL_0
#define SHUNT		ADC_CHANNEL_1

/** Reference voltage for ADC,in mv. */
#define VOLT_REF        (3300)
/** The maximal digital value */
#define MAX_DIGITAL     (4095)
/* Tracking Time*/
#define TRACKING_TIME    1
/* Transfer Period */
#define TRANSFER_PERIOD  1


void start_adc(void){
	/* Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 6.4MHz = 10 us
	 */
	adc_init(ADC, sysclk_get_cpu_hz(), 64000000, ADC_STARTUP_TIME_4);
	
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
	
	//adc_enable_channel(ADC, TMP1);
	//adc_enable_channel(ADC, TMP2);
	//adc_enable_channel(ADC, CELLV);
	//adc_enable_channel(ADC, SHUNT);
}

int ADC_READ(enum adc_channel_num_t channel) 
{
	
	adc_enable_channel(ADC, channel);
	adc_start(ADC);
	
	int raw, meas_channel;
	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY) {
			raw = adc_get_latest_value(ADC);
	}
	adc_disable_channel(ADC, channel);
	
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();
	
	
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);
	
	start_adc();
	
	/* Insert application code here, after the board has been initialized. */
	
	while(1){
		ADC_READ(TMP1);
	}
}
