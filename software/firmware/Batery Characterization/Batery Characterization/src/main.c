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
#include <math.h>



float cellV, current, tempBatt, tempFet;

void calculateCellVoltage(int rawADCData){
	cellV = (rawADCData)*((VOLT_REF/1000.0)/(4096.0*0.759));
}

void calculateCellCurrent(int rawADCData){
	current = (rawADCData)*((VOLT_REF/1000.0)/(0.005*4096.0*22.0));
}

void calculateTemp(int rawADCData, int channel){
	float voltage = (rawADCData*3.3/4096);
	float tempCalc = (1.0/((log(voltage/((VOLT_REF/1000.0)-voltage))/THERMISTOR_BETA)+(1/(THERMISTOR_BETA_TEMP+273.15))))-273.15;
	if(channel == 4)
		tempFet = tempCalc;
	else if(channel == 5)
		tempBatt = tempCalc;
}

void getADCData(){
	if((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY) {
		calculateCellVoltage(ADC->ADC_CDR[0]);
		calculateCellCurrent(ADC->ADC_CDR[1]);
		calculateTemp(ADC->ADC_CDR[4],4);
		calculateTemp(ADC->ADC_CDR[5],5);
	}
}


void TransmitTimerHandler(void){
	if(udi_cdc_is_tx_ready() && udi_cdc_get_free_tx_buffer() >= 63){
		char str[20];
		sprintf(str, "BattV: %f\n",cellV);
		udi_cdc_putc(str);
	}
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();

	/* Insert application code here, after the board has been initialized. */
	while(1){
		getADCData();
		
	}
}


