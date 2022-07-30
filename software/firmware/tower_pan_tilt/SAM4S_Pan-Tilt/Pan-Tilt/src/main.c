#include <asf.h>
#include "modbus.h"
#include "servo.h"

#define MODBUS_SLAVE_ID 2
#define MODBUS_BPS 115200
#define MODBUS_TIMEOUT 2000
#define MODBUS_SER_PORT UART1
#define MODBUS_EN_PORT PIOA
#define MODBUS_EN_PIN PIO_PA17

enum MODBUS_REGISTERS {
	CENTER_ALL = 0,
	PAN_ADJUST_POSITIVE = 1,
	PAN_ADJUST_NEGATIVE = 2,
	TILT_ADJUST_POSITIVE = 3,
	TILT_ADJUST_NEGATIVE = 4,
	HITCH_SERVO_POSITIVE = 5,
	HITCH_SERVO_NEGATIVE = 6
};


void handle_pan_tilt(servo_s *pan_servo, servo_s *tilt_servo) {
	if (intRegisters[CENTER_ALL]) {
		servo_write_angle(pan_servo, 90);
		servo_write_angle(tilt_servo, 90);
		
		intRegisters[CENTER_ALL] = 0;
	} else {
		unsigned pan_pos = pan_servo->position - intRegisters[PAN_ADJUST_POSITIVE] + intRegisters[PAN_ADJUST_NEGATIVE];
		unsigned tilt_pos = tilt_servo->position + intRegisters[TILT_ADJUST_POSITIVE] - intRegisters[TILT_ADJUST_NEGATIVE];
		servo_write_us(pan_servo, pan_pos);
		servo_write_us(tilt_servo, tilt_pos);
	
		intRegisters[PAN_ADJUST_POSITIVE] = 0;
		intRegisters[PAN_ADJUST_NEGATIVE] = 0;
		intRegisters[TILT_ADJUST_POSITIVE] = 0;
		intRegisters[TILT_ADJUST_NEGATIVE] = 0;
	}
}

void handle_hitch(servo_s *hitch_servo) {
	if (intRegisters[HITCH_SERVO_POSITIVE]) {
		servo_write_angle(hitch_servo, 60);
	} else if (intRegisters[HITCH_SERVO_NEGATIVE]) {
		servo_write_angle(hitch_servo, 120);
	}
}


int main(void) {
	sysclk_init();
	board_init();
	
	portSetup(MODBUS_SER_PORT, MODBUS_BPS, MODBUS_EN_PORT, MODBUS_EN_PIN, MODBUS_TIMEOUT);
	modbus_init(MODBUS_SLAVE_ID);
	
	servo_s pan_servo;
	servo_s tilt_servo;
	//servo_s hitch_servo;
	
	servo_setup(&pan_servo, PWM_CHANNEL_0, 1300, 3000);
	//servo_setup(&tilt_servo, PWM_CHANNEL_1, 1020, 2400);
	//servo_setup(&hitch_servo, PWM_CHANNEL_2, 0, 2000);
	
	// Center
	/*servo_write_angle(&pan_servo, 0);
	servo_write_angle(&tilt_servo, 0);
	
	// Some tests for now switching between frequencies.
	//for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	servo_write_angle(&pan_servo, 30);
	servo_write_angle(&tilt_servo, 30);
	
	//for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	servo_write_angle(&pan_servo, 60);
	servo_write_angle(&tilt_servo, 60);
	
	//for (volatile uint32_t i = 0; i < (12000000) * 3; i++);
	servo_write_angle(&pan_servo, 90);
	servo_write_angle(&tilt_servo, 90);*/
	
	servo_write_angle(&pan_servo, 0);
	servo_write_angle(&pan_servo, 30);
	servo_write_angle(&pan_servo, 60);
	servo_write_angle(&pan_servo, 90);
	servo_write_angle(&pan_servo, 120);
	servo_write_angle(&pan_servo, 150);
	servo_write_angle(&pan_servo, 180);
	
	while (1) {
		modbus_update();
		//handle_pan_tilt(&pan_servo, &tilt_servo);
		//handle_hitch(&hitch_servo);
	}
}
