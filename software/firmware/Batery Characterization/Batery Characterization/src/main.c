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

pwm_channel_t Load;

float cellV, current, tempBatt, tempFet;
float dischargeCurrent = 0;
int PWMDuty = 0;
bool updateLoop = false;
bool protectionActive = false;
float endOfDischarge = 2.7;

void calculateCellVoltage(int rawADCData){
	cellV = (rawADCData)*((VOLT_REF/1000.0)/(4096.0*0.759));
}

void calculateCellCurrent(int rawADCData){
	float voltage = (float)(rawADCData)*((float)(VOLT_REF)/1000.0)/4096.0;
	current = voltage/0.0733333;
	//current = current - 0.6;
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
	NVIC_DisableIRQ(ID_TC1);
	if(tc_get_status(TC,TC_CH) & TC_SR_CPCS){
		if(udi_cdc_get_free_tx_buffer() >= 64 && !protectionActive){
			int strln = 40;
			char str[strln];
			sprintf(str, "%f,%f,%f,%f\n\r",cellV,current,tempBatt,tempFet);
			for(int i=0;i<strln;i++)
				if(udi_cdc_is_tx_ready())
					udi_cdc_putc(str[i]);
		}
	}
	updateLoop = true;
	NVIC_EnableIRQ(ID_TC1);
}

void initPWM(){
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM,PWM_CHANNEL_0);
	pio_set_peripheral(LOAD_PORT,LOAD_PERIPH,LOAD_MASK);
	pwm_clock_t clock_setting = {
		.ul_clka = 60000000,
		.ul_clkb = 0,
		.ul_mck = 120000000
	};
	pwm_init(PWM,&clock_setting);
	Load.ul_prescaler = PWM_CMR_CPRE_CLKA;
	Load.ul_period = 3000;
	Load.ul_duty = PWMDuty;
	Load.channel = PWM_CHANNEL_0;
	Load.polarity = PWM_HIGH;
	pwm_channel_init(PWM, &Load);
	pwm_channel_enable(PWM,PWM_CHANNEL_0);
}

void checkForDischargeCurrent(){
	if(udi_cdc_is_rx_ready()){
		dischargeCurrent = (float)(udi_cdc_getc())/10.0;
	}
}


void updatePWM(int newDuty){
	Load.ul_duty = newDuty;
	pwm_channel_disable(PWM,PWM_CHANNEL_0);
	pwm_channel_init(PWM,&Load);
	pwm_channel_enable(PWM,PWM_CHANNEL_0);
}

void LoadFeedback(){
	if(dischargeCurrent > 0 && cellV <= endOfDischarge){					//under voltage protection
		dischargeCurrent = 0.0;
		PWMDuty = 0;
		updatePWM(PWMDuty);
		protectionActive = true;
	}
	if(updateLoop){
		if(dischargeCurrent - current > 0.1 && current < 25 && PWMDuty < 3000){
			updatePWM(++PWMDuty);
		}else if(current - dischargeCurrent > 0.1 && current > 0 && PWMDuty > 0){
			PWMDuty -= 1;
			updatePWM(PWMDuty);
		}
		updateLoop = false;
	}
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();
	initPWM();

	/* Insert application code here, after the board has been initialized. */
	while(1){
		getADCData();
		checkForDischargeCurrent();
		LoadFeedback();
	}
}


