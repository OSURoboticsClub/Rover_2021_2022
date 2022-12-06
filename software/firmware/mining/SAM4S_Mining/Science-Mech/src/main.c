#include <asf.h>
#include "modbus.h"
#include "stepper.h"
#include "registers.h"

int steps_per_rev = 465;

int main(void) {
	sysclk_init();
	board_init();
	
	portSetup(MODBUS_SER_PORT, MODBUS_BPS, MODBUS_EN_PORT, MODBUS_EN_PIN, MODBUS_TIMEOUT);
	modbus_init(MODBUS_SLAVE_ID);
	
	stepper_s stepper;
	stepper_setup(&stepper, STEPPER_PWM_CHANNEL, STEPPER_DIR_PORT, STEPPER_DIR_PIN, STEPPER_STEP_PORT, STEPPER_STEP_PIN);

	// Some tests for now switching between frequencies.
	//stepper_set_position(&stepper, 465);
	//for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	//stepper_set_velocity(&stepper, 10, STEPPER_DIR_CCW);
	//for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	//stepper_stop(&stepper);

	while (1) {
		modbus_update();

		if(stepper.position != intRegisters[stepperPosition]) stepper_set_position(&stepper, intRegisters[stepperPosition]);	//If the position needs to change, change it.
		
		intRegisters[limSw1] = pio_get(LSw1_PORT,PIO_TYPE_PIO_INPUT,LSw1_PIN);
		intRegisters[limSw2] = pio_get(LSw2_PORT,PIO_TYPE_PIO_INPUT,LSw2_PIN);
		intRegisters[limSw3] = pio_get(LSw3_PORT,PIO_TYPE_PIO_INPUT,LSw3_PIN);
		intRegisters[limSw4] = pio_get(LSw4_PORT,PIO_TYPE_PIO_INPUT,LSw4_PIN);
		
		if(intRegisters[lazerEnable]) pio_set(Lazer_PORT, Lazer_PIN);
		else pio_clear(Lazer_PORT, Lazer_PIN);
		
		
		if(intRegisters[cameraSelect]) pio_set(vidSel0_PORT, vidSel0_PIN);				//This is the only video select pin that matters
		else pio_clear(vidSel0_PORT, vidSel0_PIN);
	}
}