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
#include <registers.h>
#include <modbus.h>

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	
	WDT->WDT_MR |= WDT_MR_WDDIS; // Disable watchdog timer to prevent uC resetting every 15 seconds :)
	
	//Enable USB Comm port so we can send debug data over serial to a computer (could be useful)
	//Configuration for this is in conf_usb.h
	udc_start();
	modbus_init(UART1,115200,PIOA,PIO_PA17,SLAVEID);
	
}
