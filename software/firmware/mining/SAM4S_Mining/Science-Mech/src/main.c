#include <asf.h>
#include "modbus.h"
#include "stepper.h"
#include "registers.h"

//#define MODBUS_SLAVE_ID 1					//Defined in registers.h to 15
//All other definitions moved to conf_board.h in the config folder

int main(void) {
	sysclk_init();
	board_init();
	
	portSetup(MODBUS_SER_PORT, MODBUS_BPS, MODBUS_EN_PORT, MODBUS_EN_PIN, MODBUS_TIMEOUT);
	modbus_init(SLAVEID);
	
	stepper_s stepper;
	stepper_setup(&stepper, 200, STEPPER_DIR_PORT, STEPPER_DIR_PIN, STEPPER_STEP_PORT, STEPPER_STEP_PIN, STEPPER_PWM_CHANNEL);

	setPosition(&stepper,200);			//!! This is only for dev. You should see a full rotation, then a full rotation back when 

	while (1) {
		modbus_update();
		
		if( getPosition(&stepper) != intRegisters[stepperPosition]) setPosition(&stepper, intRegisters[stepperPosition]);	//If the position needs to change, change it.
		
		boolRegisters[limSw1] = pio_get(LSw1_PORT,PIO_TYPE_PIO_INPUT,LSw1_PIN);
		boolRegisters[limSw2] = pio_get(LSw2_PORT,PIO_TYPE_PIO_INPUT,LSw2_PIN);
		boolRegisters[limSw3] = pio_get(LSw3_PORT,PIO_TYPE_PIO_INPUT,LSw3_PIN);
		boolRegisters[limSw4] = pio_get(LSw4_PORT,PIO_TYPE_PIO_INPUT,LSw4_PIN);
		
		if(boolRegisters[lazerEnable]) pio_set(Lazer_PORT, Lazer_PIN);
		else pio_clear(Lazer_PORT, Lazer_PIN);
		
		
		if(intRegisters[cameraSelect]) pio_set(vidSel0_PORT, vidSel0_PIN);				//This is the only video select pin that matters
		else pio_clear(vidSel0_PORT, vidSel0_PIN);
	}
}
