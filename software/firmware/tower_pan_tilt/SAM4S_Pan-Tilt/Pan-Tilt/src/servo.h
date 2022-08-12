#ifndef SERVO_H
#define SERVO_H

#include <asf.h>


typedef struct servo_s {
	uint32_t pwm_channel_num;
	pwm_channel_t pwm_channel;
	unsigned position;
	unsigned us_min;
	unsigned us_max;
	unsigned us_center;
} servo_s;


void servo_setup(servo_s *servo, uint32_t pwm_channel_num, unsigned us_min, unsigned us_max, unsigned us_center);
void servo_write_us(servo_s *servo, unsigned us);
void servo_write_angle(servo_s *servo, unsigned angle);


#endif
