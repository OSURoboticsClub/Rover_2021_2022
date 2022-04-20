/*
 * interupt_handlers.c
 *
 * Created: 3/8/2022 10:48:45 PM
 *  Author: Anthony
 */ 

#include <asf.h>
#include <board.h>
#include <interupt_handlers.h>
#include <sleep_modes.h>




//This all might be unnecessary

void Wake_Sleep_Handler(){
	uint32_t IStatus = pio_get_interrupt_status(PIOA);
	bool USBSense = pio_get(USB_SNS_PORT,PIO_TYPE_PIO_INPUT,USB_SNS);
	bool PWRSwitchSense = pio_get(PWR_SW_PORT,PIO_TYPE_PIO_INPUT,PWR_SW);
	
	switch(IStatus){
		case(PIO_ISR_P14):
			if(USBSense){
				USBWakeUp();
				return;
			}else if(!USBSense && !PWRSwitchSense)
				goToSleep();
				return;
		case(PIO_ISR_P0):
			if(PWRSwitchSense){
				PWRSwitchWakeUp();
				return;
			}else if(!USBSense && !PWRSwitchSense)
				goToSleep();
				return;
		default:
			return;
	};
}

void Wakup_Timer_Handler(){
	pio_set(BOARD_LED_PORT,BOARD_LED);
	goToSleep();
}