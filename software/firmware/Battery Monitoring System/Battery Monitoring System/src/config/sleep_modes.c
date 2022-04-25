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
	NVIC_DisableIRQ(RTT_IRQn);
	rtt_disable_interrupt(RTT,RTT_MR_ALMIEN);
	pmc_disable_periph_clk(ID_RTT);
	
	//start shit
	//udc_start();
	
}

void PWRSwitchWakeUp(){
	//sysclk_init();
	//turn on everything
	pio_clear(NBAT_EN_PORT,NBAT_EN);
	pio_clear(RS485_NRE_PORT,RS485_NRE);
	pio_clear(RS485_DE_PORT,RS485_DE);
	pio_set(AFE_EN_PORT,AFE_EN);
	
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
	pmc_switch_mck_to_sclk(PMC_MCKR_PRES_CLK_1);
	supc_enable_backup_mode(SUPC);
	
	//pio_set(BOARD_LED_PORT,BOARD_LED);
	
}