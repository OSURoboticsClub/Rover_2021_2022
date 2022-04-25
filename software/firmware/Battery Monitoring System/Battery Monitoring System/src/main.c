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
#include <string.h>
#include <main.h>
#include <modbus.h>
#include <registers.h>
#include <interupt_handlers.h>
#include <sleep_modes.h>

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
		checkUSB();
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
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);		//Software Trigger, no freerun (I think this is defauilt)
	ADC->ADC_EMR |= ADC_EMR_TAG;					//appends channel tag to last converted data
	ADC->ADC_MR |= ADC_MR_USEQ_REG_ORDER;			//enables manual sequencer
	adc_enable_channel(ADC, CELLV_CHANNEL);
	ADC->ADC_SEQR1 |= CELLV_CHANNEL<<(4*CELLV_CHANNEL);
	adc_enable_channel(ADC, CURRENT_CHANNEL);
	ADC->ADC_SEQR1 |= CURRENT_CHANNEL<<(4*CURRENT_CHANNEL);
	adc_enable_channel(ADC, STACK_CHANNEL);
	ADC->ADC_SEQR1 |= STACK_CHANNEL<<(4*STACK_CHANNEL);
	adc_enable_channel(ADC, EXTERN_TEMP_CHANNEL);
	ADC->ADC_SEQR1 |= EXTERN_TEMP_CHANNEL<<(4*EXTERN_TEMP_CHANNEL);
	adc_enable_channel(ADC, FET_TEMP_CHANNEL);
	ADC->ADC_SEQR1 |= FET_TEMP_CHANNEL<<(4*FET_TEMP_CHANNEL);
	adc_enable_channel(ADC, SHUNT_TEMP_CHANNEL);
	ADC->ADC_SEQR1 |= SHUNT_TEMP_CHANNEL<<(4*SHUNT_TEMP_CHANNEL);
	adc_enable_interrupt(ADC,ADC_IER_DRDY);
	NVIC_EnableIRQ(ADC_IRQn);
	
	pmc_enable_periph_clk(ID_TC0);
	tc_init(TC0, ADC_TC,
			TC_CMR_WAVSEL_UP_RC
			|TC_CMR_WAVE
			| TC_CMR_TCCLKS_TIMER_CLOCK4);
	tc_write_rc(TC0,ADC_TC,18750);				  //causes timer to reset every 5ms
	tc_enable_interrupt(TC0, ADC_TC, TC_IER_CPCS);
	NVIC_EnableIRQ(TC0_IRQn);
	tc_start(TC0,ADC_TC);
}


void ADC_Handler(){
	uint32_t ADCState = ADC->ADC_ISR;
	if((ADCState & ADC_ISR_DRDY) == ADC_ISR_DRDY){
		int lastConvertedRaw = ADC->ADC_LCDR;
		int lastConvertedChannel = (lastConvertedRaw & ADC_LCDR_CHNB_Msk) >> ADC_LCDR_CHNB_Pos;
		int lastConvertedData = lastConvertedRaw & ADC_LCDR_LDATA_Msk;
		
		switch(lastConvertedChannel){
		
			case CELLV_CHANNEL:
				rawADCData[CELLVIDXS + currentExternCell] = lastConvertedData;
				needsProcessing[CELLVIDXS + currentExternCell] = true;
				if(currentExternCell == 5) currentExternCell = 0;
				else currentExternCell++;
				pinWrite(CELL_SEL0_PORT,CELL_SEL0,(bool)(currentExternCell&1));
				pinWrite(CELL_SEL1_PORT,CELL_SEL1,(bool)(currentExternCell&2));
				pinWrite(CELL_SEL2_PORT,CELL_SEL2,(bool)(currentExternCell&4));
				break;
			
			case CURRENT_CHANNEL:
				rawADCData[CURRENTIDX] = adc_get_channel_value(ADC,CURRENT_CHANNEL);
				needsProcessing[CURRENTIDX] = true;
				break;
			
			case STACK_CHANNEL:
				rawADCData[STACKVOLTIDX] = adc_get_channel_value(ADC,STACK_CHANNEL);
				needsProcessing[STACKVOLTIDX] = true;
				break;
			
			case EXTERN_TEMP_CHANNEL:
				rawADCData[TEMPIDXS + currentExternTempSensor] = adc_get_channel_value(ADC,EXTERN_TEMP_CHANNEL);
				needsProcessing[TEMPIDXS + currentExternTempSensor] = true;
				if(currentExternTempSensor == 5) currentExternTempSensor = 0;
				else currentExternTempSensor++;
				pinWrite(TEMP_SEL0_PORT,TEMP_SEL0,(bool)(currentExternTempSensor&1));
				pinWrite(TEMP_SEL1_PORT,TEMP_SEL1,(bool)(currentExternTempSensor&2));
				pinWrite(TEMP_SEL2_PORT,TEMP_SEL2,(bool)(currentExternTempSensor&4));
				break;
		
			case FET_TEMP_CHANNEL:
				rawADCData[FETTEMPIDX] = adc_get_channel_value(ADC,FET_TEMP_CHANNEL);
				needsProcessing[FETTEMPIDX] = true;
				break;
			
			case SHUNT_TEMP_CHANNEL:
				rawADCData[SHUNTTEMPIDX] = adc_get_channel_value(ADC,SHUNT_TEMP_CHANNEL);
				needsProcessing[SHUNTTEMPIDX] = true;
				break;
		};
	}
}

void analogCalculate(){
	if( needsProcessing[CELL1IDX] || needsProcessing[CELL2IDX] || needsProcessing[CELL3IDX] || needsProcessing[CELL4IDX] || needsProcessing[CELL5IDX] || needsProcessing[CELL6IDX]){
		int processIDX;
		for(int i=CELL1IDX;i<=CELL6IDX;i++) if(needsProcessing[i]) processIDX = i;
		processedADCData[processIDX] = (3.3*(rawADCData[processIDX]/4095.0)) * (1.0/0.759);
		needsProcessing[processIDX] = false;
	}
	if(needsProcessing[TEMP1IDX] || needsProcessing[TEMP2IDX] || needsProcessing[TEMP3IDX] || needsProcessing[TEMP4IDX] || needsProcessing[TEMP5IDX] || needsProcessing[TEMP6IDX]){
		int processIDX;
		for(int i=TEMP1IDX;i<=TEMP6IDX;i++) if(needsProcessing[i]) processIDX = i;
		processedADCData[processIDX] = ExtTempLookUpTable[(int)(rawADCData[processIDX]/4)];
		needsProcessing[processIDX] = false;
	}
	if(needsProcessing[CURRENTIDX]){
		processedADCData[CURRENTIDX] = (3.3*(rawADCData[CURRENTIDX]/4095.0)) * (1000/20);
		needsProcessing[CURRENTIDX] = false;
	}
	if(needsProcessing[SHUNTTEMPIDX]){
		processedADCData[SHUNTTEMPIDX] = PCBTempLookUpTable[(int)(rawADCData[SHUNTTEMPIDX]/4)];
		needsProcessing[SHUNTTEMPIDX] = false;
	}
	if(needsProcessing[FETTEMPIDX]){
		processedADCData[FETTEMPIDX] = PCBTempLookUpTable[(int)(rawADCData[FETTEMPIDX]/4)];
		needsProcessing[FETTEMPIDX] = false;
	}
	if(needsProcessing[STACKVOLTIDX]){
		processedADCData[STACKVOLTIDX] = (rawADCData[STACKVOLTIDX]/4095.0)*33.0;
		needsProcessing[STACKVOLTIDX] = false;
	}
}



/****************************
*	Protections
*****************************/

bool activeProtections[QTY_PROTECTIONS];
bool batteryStable = true, ignoreProtections = false;
bool protected = false;

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
	if(!batteryStable && !ignoreProtections && !protected){
		protec();
		protected = true;
	}else if(batteryStable)
		ignoreProtections = false;
		//pio_clear(BOARD_LED_PORT,BOARD_LED);
}

bool overTemp(){
	for(int i=TEMP1IDX;i<=TEMP6IDX;i++)
		if(processedADCData[i] >= CELL_OVER_TEMP_THRESHOLD)
			return true;
	if(processedADCData[SHUNTTEMPIDX] >= PCB_OVER_TEMP_THRESHOLD || processedADCData[FETTEMPIDX] >= PCB_OVER_TEMP_THRESHOLD)
		return true;
	return false;
}
bool underTemp(){
	for(int i=TEMP1IDX;i<=TEMP6IDX;i++)
		if(processedADCData[i] <= UNDER_TEMP_THRESHOLD)
			return true;
	return false;
}
bool overVolt(){
	for(int i=CELL1IDX;i<=CELL6IDX;i++)
		if(processedADCData[i] >= OVER_VOLTAGE_THRESHOLD)
			return true;
	return false;
}
bool underVolt(){
	for(int i=CELL1IDX;i<=CELL6IDX;i++)
		if(processedADCData[i] <= UNDER_VOLTAGE_THRESHOLD)
			return true;
	return false;
}
bool overCurrent(){
	if(processedADCData[CURRENTIDX] >= OVER_CURRENT_THRESHOLD)
		return true;
	return false;
}
bool cellImbalance(){
	float minCell = 5.0, maxCell = 0;
	for(int i=CELL1IDX;i<=CELL6IDX;i++){
		if(processedADCData[i] < minCell) minCell = processedADCData[i];
		if(processedADCData[i] > maxCell) maxCell = processedADCData[i];
	}
	if((maxCell - minCell) > CELL_IMBALANCE_THRESHOLD)
		return true;
	return false;
}

void protec(){
	
	pio_set(NBAT_EN_PORT, NBAT_EN);
	
	pmc_enable_periph_clk(ID_TC1);
	tc_init(TC0, BLINK_TC,
			TC_CMR_WAVSEL_UP_RC
			|TC_CMR_WAVE
			| TC_CMR_TCCLKS_TIMER_CLOCK5);
	tc_write_rc(TC0,BLINK_TC,16000);
	tc_enable_interrupt(TC0, BLINK_TC, TC_IER_CPCS);
	tc_start(TC0,BLINK_TC);
	NVIC_EnableIRQ(TC1_IRQn);
	

}



/****************************
*	USB COMMS
*****************************/

void checkUSB(){
	if(udi_cdc_get_nb_received_data() > 0){
		char inChar = udi_cdc_getc();
		if (inChar == 0) return;
		if(inChar == 's')
			printStatus();
		else if(inChar == 'c'){
			clearProtections();
			char * message = "\n\rAll protections will be ignored till the battery is stable again\n\r";
			printString(message);
		}else
			printInstructions();
	}
}

void printStatus(){
	char str[256] = "\n\rName, Current Value, Smallest lifetime value, largest lifetime value\n\r";
	printString(str);
	for (int i=0;i<6;i++){
		sprintf(str, "Cell_%i_voltage,%f,%f,%f\n\r",i+1,processedADCData[CELLVIDXS+i],0.0,0.0);
		printString(str);
	}
	for (int i=0;i<6;i++){
		sprintf(str, "Ext_Temp_%i,%f,%f,%f\n\r",i+1,processedADCData[TEMPIDXS+i],0.0,0.0);
		printString(str);
	}
	sprintf(str, "Current,%f,%f,%f\n\r",processedADCData[CURRENTIDX],0.0,0.0);
	printString(str);
	sprintf(str, "Shunt_Temp,%f,%f,%f\n\r",processedADCData[SHUNTTEMPIDX],0.0,0.0);
	printString(str);
	sprintf(str, "Fet_Temp,%f,%f,%f\n\r",processedADCData[FETTEMPIDX],0.0,0.0);
	printString(str);
	sprintf(str, "Stack_Voltage,%f,%f,%f\n\r",processedADCData[STACKVOLTIDX],0.0,0.0);
	printString(str);
}

void printInstructions(){
	char * instructions = "\n\rPlease Type\"s\" to get the current status of the battery\n\rAfter determining the best action type \"c\" to temporarily clear all protections\n\r[WARNING] when the protections are cleared, the BMS will ignore all protections till the battery is stable once again\n\r This is indicated by the red LED turning off.\n\r";
	printString(instructions);
}

void clearProtections(){
	ignoreProtections = true;
	pio_clear(NBAT_EN_PORT, NBAT_EN);
	tc_stop(TC0,BLINK_TC);
	pio_set(BOARD_LED_PORT,BOARD_LED);
}



/****************************
*	MISC
*****************************/
void pinWrite(Pio* port, const uint32_t pin_mask, bool logic){
	if(logic) pio_set(port,pin_mask);
	else pio_clear(port,pin_mask);
}


void TC0_Handler(){
	if( (tc_get_status(TC0,ADC_TC) & TC_SR_CPCS) == TC_SR_CPCS)
		adc_start(ADC);
}

void TC1_Handler(){
	if( (tc_get_status(TC0,BLINK_TC) & TC_SR_CPCS) == TC_SR_CPCS)
		pio_toggle_pin_group(BOARD_LED_PORT,BOARD_LED);
}

void printString(char* str){
	for(int i=0;i<strlen(str);i++){
		if(udi_cdc_is_tx_ready())
			udi_cdc_putc(str[i]);
	}
}
