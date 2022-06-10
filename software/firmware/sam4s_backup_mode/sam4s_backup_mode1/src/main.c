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
#include <stdio.h>
#include <string.h>

// Pin defines
#define TEST_LED				PIO_PA3_IDX
#define NBAT_EN					PIO_PA11_IDX
#define RS485_NRE				PIO_PA12_IDX
#define RS485_DE				PIO_PA13_IDX
#define AFE_EN					PIO_PA7_IDX
#define USB_SNS					PIO_PA14_IDX

void led_setup(void);
void rtt_setup(void);

int main (void)
{
	
	/* Insert system clock initialization code here (sysclk_init()). */
	board_init();
	sysclk_init();
	
	// Disable watchdog timer to stop the MCU from resetting
	WDT->WDT_MR |= WDT_MR_WDDIS;
	
	/* Insert application code here, after the board has been initialized. */
	ioport_init();
	
	// Call setup routines (leaves LED off)
	led_setup();
	
	// Turn off all unusued stuff via I/O control pins
	ioport_set_pin_dir(USB_SNS, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(NBAT_EN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(NBAT_EN, 1);
	ioport_set_pin_dir(RS485_NRE, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(RS485_NRE, 1);
	ioport_set_pin_dir(RS485_DE, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(RS485_DE, 0);
	ioport_set_pin_dir(AFE_EN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(AFE_EN, 0);
	
	// These pins would source current if HIGH, so setting them as inputs
	ioport_set_pin_dir(PIO_PA0_IDX, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(PIO_PB0_IDX, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(PIO_PB1_IDX, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(PIO_PB7_IDX, IOPORT_DIR_INPUT);
	
	// Set up RTT
	rtt_setup();
	
	// Set wakeup mode to use RTT alarm
	supc_set_wakeup_mode(SUPC, SUPC_WUMR_RTTEN);
	
	// Switch to slow clock
	pmc_switch_mck_to_sclk(PMC_MCKR_PRES_CLK_1);
	
	// Disable fastrc osicllator
	pmc_osc_disable_fastrc();
	
	// Disable PLL_A
	pmc_disable_pllack();
		
	// Go into backup mode (MCU will wakeup, go to RTT interrupt, then reset)
	supc_enable_backup_mode(SUPC);
	
	for(;;)
	{
		
	}
	
}

/* USER DEFINED FUNCTIONS */

// Sets up the LED
void led_setup(void)
{
	// Configure TEST_LED I/O pin and set high
	ioport_set_pin_dir(TEST_LED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(TEST_LED, 0);
}

// Sets up RTT
void rtt_setup(void)
{
	// Enable interrupt
	NVIC_EnableIRQ(RTT_IRQn);
	
	// Setup RTT to run at 1hz from RTC
	rtt_init(RTT, RTT_MR_RTC1HZ);
	
	// 10 seconds? (it's more like 13?)
	rtt_write_alarm_time(RTT, 4);
	
	// Enable RTT interrupt
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}

/* INTERRUPTS */

// This interrupt handler is entered when the RTT wakes up the MCU
void RTT_Handler( void) {
    // reading status register will clear interrupt flags
    uint32_t status = rtt_get_status(RTT);
	
    if ((status & RTT_SR_ALMS) >= 1){//ALMS generated an interrupt		
        /*
        Run the Code that you want to run every interrupt
        */
		
		// This LED stuff is just in here to show the MCU woke up
		ioport_toggle_pin_level(TEST_LED); // Turn LED on
		delay_ms(1000);
		ioport_toggle_pin_level(TEST_LED); // Turn LED off
		delay_ms(1000);
		ioport_toggle_pin_level(TEST_LED); // Turn LED on
		delay_ms(1000);
		ioport_toggle_pin_level(TEST_LED); // Turn LED off
		delay_ms(1000);
		ioport_toggle_pin_level(TEST_LED); // Turn LED on
		delay_ms(1000);
		ioport_toggle_pin_level(TEST_LED); // Turn LED off
		delay_ms(1000);
		
		/* Technically this bottom part isn't needed because the MCU resets after exiting this interrupt routine
		However, if this interrupt was triggered while the MCU was not in backup mode, this below section would
		set it up to interrupt after the same time period again.*/
		
        // reset RTT counter
        REG_RTT_MR |= RTT_MR_RTTRST;
		
        // upon interrupt REG_RTT_AR.RTT_AR_ALMV gets set to the maximum value
        // we need to disable alarm to set a new value in ALMV
        REG_RTT_MR &= ~RTT_MR_ALMIEN;
		rtt_write_alarm_time(RTT, 4);  // 10 seconds? (it's more like 13?)
		
        //turn interrupt back on
        REG_RTT_MR |= RTT_MR_ALMIEN;
	}	
}