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
	WDT->WDT_MR |= WDT_MR_WDDIS; // Disable watchdog timer to prevent uC resetting every 15 seconds :)
	
	//Enable USB Comm port so we can send debug data over serial to a computer (could be useful)
	//Configuration for this is in conf_usb.h
	udc_start();
	
	pmc_enable_periph_clk(ID_PIOA);			//This enables GPIO Outpus, (necessary)
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_input(LSw1_PORT,LSw1_PIN,PIO_DEBOUNCE);
	pio_set_input(LSw2_PORT,LSw2_PIN,PIO_DEBOUNCE);
	pio_set_input(LSw3_PORT,LSw3_PIN,PIO_DEBOUNCE);
	pio_set_input(LSw4_PORT,LSw4_PIN,PIO_DEBOUNCE);
	
	pio_set_output(Lazer_PORT,Lazer_PIN,LOW,DISABLE,DISABLE);
	pio_set_output(vidSel0_PORT,vidSel0_PIN,LOW,DISABLE,DISABLE);
	pio_set_output(vidSel1_PORT,vidSel1_PIN,LOW,DISABLE,DISABLE);
}
