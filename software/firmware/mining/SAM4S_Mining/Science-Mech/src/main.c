#include <asf.h>
#include "modbus.h"
#include "stepper.h"

#define MODBUS_SLAVE_ID 1
#define MODBUS_BPS 115200
#define MODBUS_TIMEOUT 2000
#define MODBUS_SER_PORT UART0
#define MODBUS_EN_PORT PIOA
#define MODBUS_EN_PIN PIO_PA11

#define STEPPER_PWM_CHANNEL PWM_CHANNEL_3
#define STEPPER_DIR_PORT PIOA
#define STEPPER_DIR_PIN PIO_PA8 // PA6 not working on breakout

int main(void) {
	sysclk_init();
	board_init();
	
	portSetup(MODBUS_SER_PORT, MODBUS_BPS, MODBUS_EN_PORT, MODBUS_EN_PIN, MODBUS_TIMEOUT);
	modbus_init(MODBUS_SLAVE_ID);
	
	stepper_s stepper;
	stepper_setup(&stepper, STEPPER_PWM_CHANNEL, STEPPER_DIR_PORT, STEPPER_DIR_PIN);
	
	// Some tests for now switching between frequencies.
	for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	stepper_set_velocity(&stepper, 2000, STEPPER_DIR_CW);
	for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	stepper_set_velocity(&stepper, 10, STEPPER_DIR_CCW);
	for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	stepper_stop(&stepper);

	while (1) {
		modbus_update();
	}
}
