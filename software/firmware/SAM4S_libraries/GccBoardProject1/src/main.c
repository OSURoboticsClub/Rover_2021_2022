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

//#include "pio.h"
#include "conf_clock.h"

#define 	ADC_CLOCK   1000000

pwm_channel_t pwm_data_instance;
Spi spi_data_instance;
uint32_t adc_data;

uint8_t ADC_IrqHandler(void)
{
	// Check the ADC conversion status
	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY)
	{
		// Get latest digital data value from ADC and can be used by application
		adc_data = adc_get_latest_value(ADC);
		return 1;
	}
	return 0;
}

void adc_setup(void)
{
	adc_init(ADC, SYS_CLK_FREQ, ADC_CLOCK, 8);
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_enable_channel(ADC, ADC_CHANNEL_1);
	adc_enable_interrupt(ADC, ADC_IER_DRDY);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);
}

// Probably only want to run this once at the start of program
// Configures the pwm peripheral clocks which are used in the peripheral
void pwm_configure_clk(uint32_t clka_freq, uint32_t clkb_freq) {
	
	// Must be done to send a clock signal to the peripheral
	pmc_enable_periph_clk(ID_PWM);
	
	// There are 2 peripheral clocks which are used in the PWM system. (A and B)
	pwm_clock_t clock_setting = {
		.ul_clka = clka_freq,
		.ul_clkb = clkb_freq,
		.ul_mck = SYS_CLK_FREQ	// Using a #define here for the system clock frequency. I think this is fine for now. 
								//May want to move the #define into the conf_clock.h file provided by ASF
	};
	
	pwm_init(PWM, &clock_setting);
}

void pwm_configure_channel(uint32_t channel, Pio * port, uint32_t pin, const pio_type_t peripheral, float duty, uint32_t clk_period) {
	
	pwm_channel_disable(PWM, channel);
	
	// This line might not be necessary
	//pio_set_output(port, pin, LOW, DISABLE, DISABLE);
	
	pio_set_peripheral(port, peripheral, pin);
	
	pwm_data_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_data_instance.ul_period = clk_period;
	pwm_data_instance.ul_duty = duty*clk_period;
	pwm_data_instance.channel = channel; // There are predefined constants for PWM channels, and they should directly map to 0-3.
	pwm_channel_init(PWM, &pwm_data_instance);
	
	
}

void pwm_update_duty(uint32_t channel, float duty, uint32_t clk_period) {
	
	// Disable channel before changing anything. May not need to do this. More testing is needed
	
	// TODO: Figure out if this is necessary
	
	pwm_data_instance.ul_period = clk_period;
	pwm_data_instance.ul_duty = duty*clk_period;
	pwm_data_instance.channel = channel;
	// Updates specified pwm channel with new duty cycle
	
	pwm_channel_disable(PWM, channel);
	pwm_channel_init(PWM, &pwm_data_instance);
	
	pwm_channel_enable(PWM, channel);
	
}

/*
void spi_master_init(Spi *p_spi)
{
	spi_enable_clock(p_spi);
	spi_reset(p_spi);
	spi_set_master_mode(p_spi);
	spi_disable_mode_fault_detect(p_spi);
	spi_disable_loopback(p_spi);
	spi_set_peripheral_chip_select_value(p_spi,
	spi_get_pcs(DEFAULT_CHIP_ID));
	spi_set_fixed_peripheral_select(p_spi);
	spi_disable_peripheral_select_decode(p_spi);
	spi_set_delay_between_chip_select(p_spi, CONFIG_SPI_MASTER_DELAY_BCS);
}

void spi_master_setup_device(Spi *p_spi, struct spi_device *device,
spi_flags_t flags, uint32_t baud_rate, board_spi_select_id_t sel_id)
{
	spi_set_transfer_delay(p_spi, device->id, CONFIG_SPI_MASTER_DELAY_BS,
	CONFIG_SPI_MASTER_DELAY_BCT);
	spi_set_bits_per_transfer(p_spi, device->id, CONFIG_SPI_MASTER_BITS_PER_TRANSFER);
	spi_set_baudrate_div(p_spi, device->id,
	spi_calc_baudrate_div(baud_rate, sysclk_get_peripheral_hz()));
	spi_configure_cs_behavior(p_spi, device->id, SPI_CS_KEEP_LOW);
	spi_set_clock_polarity(p_spi, device->id, flags >> 1);
	spi_set_clock_phase(p_spi, device->id, ((flags & 0x1) ^ 0x1));
}
*/

void gpio_example(void) {
	pio_set_output(PIOB, PIO_PB14, LOW, ENABLE, ENABLE);
	pio_clear(PIOB, PIO_PB14);
}

void pwm_example(void) {
	pwm_configure_clk(100000, 0);
	
	pwm_configure_channel(PWM_CHANNEL_0, PIOB, PIO_PB0, PIO_PERIPH_A, 0.5, 100);
	pwm_configure_channel(PWM_CHANNEL_0, PIOA, PIO_PA19, PIO_PERIPH_B, 0.5, 100);
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	//pmc_enable_interrupt(ID_ADC);
	//pmc_disable_periph_clk(ID_WDT);

	// Should be moving initialization stuff to this function
	board_init();
	sysclk_init();
	
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_ADC);
	pio_set_output(PIOB, PIO_PB14, LOW, DISABLE, DISABLE);
	pio_set_output(PIOA, PIO_PA22, LOW, DISABLE, DISABLE);
	pio_clear(PIOB, PIO_PB14);
	
	pio_set_input(PIOA, PIO_PA19, 0);
	adc_setup();

	// Disable watchdog timer to stop the MCU from resetting
	WDT->WDT_MR |= WDT_MR_WDDIS;
	
	pwm_configure_clk(100000, 0);
	
	pwm_configure_channel(PWM_CHANNEL_0, PIOB, PIO_PB0, PIO_PERIPH_A, 0.5, 100);
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
	
	
	uint32_t c = 0;
	uint8_t x = 0;
	uint16_t period = 100;
	float duty;
	adc_start(ADC);
	
	for(;;) {
		c++;
		
		if(c > 0x00020000) {
			c = 0;
			//if(1) {
			if(ADC_IrqHandler() == 1) {
				if((PIOA->PIO_PDSR & (PIO_PA19)) > 0) {
					duty = 0.1;
				} else {
					duty = ((float) (adc_data / 16.0)) / 256.0;
				}
				
				//period = ((adc_data / 16.0));
				if(duty < 0.05) {duty = 0.0;}
				if(duty > 1.0) {duty = 1.0;}
				//x++;
				//x %= 100;
				//period = 50 + x;
				
				//duty = 1.0;
				pwm_update_duty(PWM_CHANNEL_0, duty, period);
				adc_start(ADC);
			}
		}
	}
}