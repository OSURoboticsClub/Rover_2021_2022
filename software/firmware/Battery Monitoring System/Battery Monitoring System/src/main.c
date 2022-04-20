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
#include <modbus.h>
#include <registers.h>
#include <interupt_handlers.h>
#include <sleep_modes.h>
#include <main.h>

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	
	sysclk_init();
	board_init();
	adcSetup();

	/* Insert application code here, after the board has been initialized. */
	
	while(true){
		modbus_update();
		analogCalculate();
		triggerProtections();
	}
}


/****************************
*	ADC and data processing
*****************************/

int rawADCData[QTY_ANALOG_SOURCES];
float processedADCData[QTY_ANALOG_SOURCES];
bool needsProcessing[QTY_ANALOG_SOURCES];
int currentExternTempSensor = 0;
int currentExternCell		= 0;
float PCBTempLookUpTable[1024];
float ExtTempLookUpTable[1024];

void adcSetup(){
	//Generate temperature lookup tables.
	float PCBBeta = 4700.0;
	float ExBeta = 3950.0;
	for(int i=0;i<1024;i++){
		float voltage = ((float)(i)/1023)*3.3;
		PCBTempLookUpTable[i] = (1.0/((log(voltage/(3.3-voltage))/PCBBeta)+(1.0/(25.0+273.15))))-273.15;
		ExtTempLookUpTable[i] = (1.0/((log(voltage/(3.3-voltage))/ExBeta)+(1.0/(25.0+273.15))))-273.15;
	}
	
	//enable and start adc.
	pmc_enable_periph_clk(ID_ADC);
	adc_init(ADC,sysclk_get_peripheral_hz(),ADC_CLK_FREQ,ADC_STARTUP_TIME_4);
	adc_configure_timing(ADC, QTY_TRACKING_PERIODS, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);
	adc_enable_channel(ADC, CELLV_CHANNEL);
	adc_enable_channel(ADC, CURRENT_CHANNEL);
	adc_enable_channel(ADC, STACK_CHANNEL);
	adc_enable_channel(ADC, EXTERN_TEMP_CHANNEL);
	adc_enable_channel(ADC, FET_TEMP_CHANNEL);
	adc_enable_channel(ADC, SHUNT_TEMP_CHANNEL);
	adc_enable_interrupt(ADC,ADC_IER_DRDY);
	NVIC_EnableIRQ(ADC_IRQn);
	adc_start(ADC);
}

void ADC_Handler(){
	if((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY){
		rawADCData[CELLVIDXS + currentExternCell] = ADC->ADC_CDR[CELLV_CHANNEL];
		needsProcessing[CELLVIDXS + currentExternCell] = true;
		if(currentExternCell == 5) currentExternCell = 0;
		else currentExternCell++;
		pinWrite(CELL_SEL0_PORT,CELL_SEL0,(bool)(currentExternCell&1));
		pinWrite(CELL_SEL1_PORT,CELL_SEL1,(bool)(currentExternCell&2));
		pinWrite(CELL_SEL2_PORT,CELL_SEL2,(bool)(currentExternCell&4));
		rawADCData[CURRENTIDX] = ADC->ADC_CDR[CURRENT_CHANNEL];
		needsProcessing[CURRENTIDX] = true;
		rawADCData[STACKVOLTIDX] = ADC->ADC_CDR[STACK_CHANNEL];
		needsProcessing[STACKVOLTIDX] = true;
		rawADCData[TEMPIDXS + currentExternTempSensor] = ADC->ADC_CDR[EXTERN_TEMP_CHANNEL];
		needsProcessing[TEMPIDXS + currentExternTempSensor] = true;
		if(currentExternTempSensor == 5) currentExternTempSensor = 0;
		else currentExternTempSensor++;
		pinWrite(TEMP_SEL0_PORT,TEMP_SEL0,(bool)(currentExternTempSensor&1));
		pinWrite(TEMP_SEL1_PORT,TEMP_SEL1,(bool)(currentExternTempSensor&2));
		pinWrite(TEMP_SEL2_PORT,TEMP_SEL2,(bool)(currentExternTempSensor&4));
		rawADCData[FETTEMPIDX] = ADC->ADC_CDR[FET_TEMP_CHANNEL];
		needsProcessing[FETTEMPIDX] = true;
		rawADCData[SHUNTTEMPIDX] = ADC->ADC_CDR[SHUNT_TEMP_CHANNEL];
		needsProcessing[SHUNTTEMPIDX] = true;
        adc_start(ADC);
	}
}

void analogCalculate(){
	if( needsProcessing[CELL1IDX] || needsProcessing[CELL2IDX] || needsProcessing[CELL3IDX] || needsProcessing[CELL4IDX] || needsProcessing[CELL5IDX] || needsProcessing[CELL6IDX]){
		int processIDX;
		for(int i=CELL1IDX;i<=CELL6IDX;i++) if(needsProcessing[i]) processIDX = i;
		processedADCData[processIDX] = (3.3*(rawADCData[processIDX]/4095.0)) * (1.0/0.759);
	}
	else if(needsProcessing[TEMP1IDX] || needsProcessing[TEMP2IDX] || needsProcessing[TEMP3IDX] || needsProcessing[TEMP4IDX] || needsProcessing[TEMP5IDX] || needsProcessing[TEMP6IDX]){
		int processIDX;
		for(int i=TEMP1IDX;i<=TEMP6IDX;i++) if(needsProcessing[i]) processIDX = i;
		processedADCData[processIDX] = ExtTempLookUpTable[(int)(rawADCData[processIDX]/4)];
		needsProcessing[processIDX] = false;
	}
	else if(needsProcessing[CURRENTIDX]){
		processedADCData[CURRENTIDX] = (3.3*(rawADCData[CURRENTIDX]/4095.0)) * (1000/20);
		needsProcessing[CURRENTIDX] = false;
	}
	else if(needsProcessing[SHUNTTEMPIDX]){
		processedADCData[SHUNTTEMPIDX] = PCBTempLookUpTable[(int)(rawADCData[SHUNTTEMPIDX]/4)];
		needsProcessing[SHUNTTEMPIDX] = false;
	}
	else if(needsProcessing[FETTEMPIDX]){
		processedADCData[FETTEMPIDX] = PCBTempLookUpTable[(int)(rawADCData[FETTEMPIDX]/4)];
		needsProcessing[FETTEMPIDX] = false;
	}
	else if(needsProcessing[STACKVOLTIDX]){
		processedADCData[STACKVOLTIDX] = (rawADCData[STACKVOLTIDX]/4095.0)*33.0;
		needsProcessing[STACKVOLTIDX] = false;
	}
}



/****************************
*	Protections
*****************************/

bool activeProtections[QTY_PROTECTIONS];
bool batteryStable = true, ignoreProtections = false;

void triggerProtections(){
	if(overTemp())
		activeProtections[OVERTEMPIDX] = true;
	if(underTemp())
		activeProtections[UNDERTEMPIDX] = true;
	if(overVolt())
		activeProtections[OVERVOLTIDX] = true;
	if(underVolt())
		activeProtections[UNDERVOLTIDX] = true;
	if(overCurrent())
		activeProtections[OVERCURRENTIDX] = true;
	if(cellImbalance())
		activeProtections[CELLIMBALANCEIDX] = true;
	batteryStable = !(activeProtections[OVERTEMPIDX] || activeProtections[UNDERTEMPIDX] || activeProtections[OVERVOLTIDX] || activeProtections[UNDERVOLTIDX] || activeProtections[OVERCURRENTIDX] || activeProtections[CELLIMBALANCEIDX]);
	if(!batteryStable && !ignoreProtections)
		protec();
	else if(batteryStable)
		ignoreProtections = false;
		pio_clear(BOARD_LED_PORT,BOARD_LED);
}

bool overTemp(){
	
}
bool underTemp(){
	
}
bool overVolt(){
	
}
bool underVolt(){
	
}
bool overCurrent(){
	
}
bool cellImbalance(){
	
}

void protec(){
	pio_set(NBAT_EN_PORT, NBAT_EN);
	
	pmc_enable_periph_clk(ID_TC1);
	tc_init(TC0, TC_CH,
			TC_CMR_WAVE
			| TC_CMR_WAVSEL_UP_RC
			| TC_CMR_TCCLKS_TIMER_CLOCK4);
	tc_enable_interrupt(TC0, TC_CH, TC_IER_CPCS);
	NVIC_EnableIRQ(TC1_IRQn);
	tc_write_rc(TC0,TC_CH,468750);				  //causes timer to reset every 500ms (exactly)
	tc_start(TC0,TC_CH);
	
	udc_start();
}

void clearProtections(){
	ignoreProtections = true;
	pio_clear(NBAT_EN_PORT, NBAT_EN);
	tc_stop(TC0,TC_CH);
	pmc_disable_periph_clk(ID_TC1);
	pio_set(BOARD_LED_PORT,BOARD_LED);
}



/****************************
*	MISC
*****************************/
void pinWrite(Pio* port, const uint32_t pin_mask, bool logic){
	if(logic) pio_set(port,pin_mask);
	else pio_clear(port,pin_mask);
}

void TC1_Handler(){
	pio_toggle_pin(BOARD_LED);
}