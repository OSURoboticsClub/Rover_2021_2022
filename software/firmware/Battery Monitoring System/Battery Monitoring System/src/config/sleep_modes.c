/*
 * sleep_modes.c
 *
 * Created: 3/8/2022 10:44:21 PM
 *  Author: Anthony
 */ 

#include <asf.h>
#include <board.h>
#include <sleep_modes.h>

void USBWakeUp(){
	//Get out of sleep mode
	
	//disable RTT
	
	//enable all periph clocks
	
	//start shit
	udc_start();
	
	
	
}

void PWRSwitchWakeUp(){
	
}


void goToSleep(){
	//turn off everything
	pio_set(NBAT_EN_PORT,NBAT_EN);
	pio_set(RS485_NRE_PORT,RS485_NRE);
	pio_clear(RS485_DE_PORT,RS485_DE);
	pio_clear(AFE_EN_PORT,AFE_EN);
	pio_clear(BOARD_LED_PORT,BOARD_LED);
	
	
	//enable RTT for periodic wakeups
	NVIC_EnableIRQ(RTT_IRQn);
	pmc_enable_periph_clk(ID_RTT);
	rtt_write_alarm_time(RTT,PERIODIC_WAKEUP_TIME);
	rtt_init(RTT,RTT_MR_ALMIEN);
	rtt_enable_interrupt(RTT,RTT_MR_ALMIEN);
	
	//enter sleep mode
	pmc_enable_periph_clk(ID_SUPC);
	supc_set_wakeup_mode(SUPC,SUPC_WUMR_RTTEN_ENABLE | SUPC_WUMR_WKUPDBC_512_SCLK);
	supc_set_wakeup_inputs(SUPC,SUPC_WUIR_WKUPEN0 | SUPC_WUIR_WKUPEN8 , SUPC_WUIR_WKUPT0 | SUPC_WUIR_WKUPT8);
	//supc_disable_voltage_regulator(SUPC);
	
	//pio_set(BOARD_LED_PORT,BOARD_LED);
	
}