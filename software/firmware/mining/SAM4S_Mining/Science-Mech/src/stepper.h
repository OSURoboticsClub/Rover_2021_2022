#ifndef STEPPER_H
#define STEPPER_H

#include <asf.h>

typedef enum stepper_dir {
	STEPPER_DIR_CCW,
	STEPPER_DIR_CW
} stepper_dir;


typedef struct stepper_s {
	uint32_t pwm_channel_num;
	pwm_channel_t pwm_channel;
	Pio *dir_port;
	uint32_t dir_pin;
} stepper_s;

void stepper_setup(stepper_s *stepper, uint32_t pwm_channel_num, Pio *dir_port, uint32_t dir_pin);
void stepper_set_velocity(stepper_s *stepper, unsigned steps_per_sec, const stepper_dir dir);
void stepper_stop(stepper_s *stepper);


#endif
