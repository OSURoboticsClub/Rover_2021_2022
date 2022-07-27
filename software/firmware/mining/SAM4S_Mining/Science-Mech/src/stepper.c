#include "stepper.h"

/* The higher this is, the more accurate the PWM.
 * However, the higher it is means the lower max frequency supported due to overflow.
 */
#define PWM_CLK_SPEED 50000

// Percentage of duty cycle, ie 0.5 = 50% duty cycle
#define PWM_DUTY_CYCLE 0.5

static void _init_pwm(stepper_s *stepper) {
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, stepper->pwm_channel_num);
	
	pwm_clock_t clock_setting = {
		.ul_clka = PWM_CLK_SPEED,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_peripheral_bus_hz(PWM)
	};
	pwm_init(PWM, &clock_setting);
}

static void _pwm_set_freq(stepper_s *stepper, unsigned freq) {
	int period = PWM_CLK_SPEED / (freq <= PWM_CLK_SPEED ? freq : PWM_CLK_SPEED);
	
	pwm_channel_t pwm_channel_instance = {
		.ul_prescaler = PWM_CMR_CPRE_CLKA,
		.ul_period = period,
		.ul_duty = period * PWM_DUTY_CYCLE,
		.channel = stepper->pwm_channel_num
	};
	stepper->pwm_channel = pwm_channel_instance;
	
	pwm_channel_disable(PWM, stepper->pwm_channel_num);
	pwm_channel_init(PWM, &stepper->pwm_channel);
	pwm_channel_enable(PWM, stepper->pwm_channel_num);
}

void stepper_setup(stepper_s *stepper, uint32_t pwm_channel_num, Pio *dir_port, uint32_t dir_pin) {
	stepper->pwm_channel_num = pwm_channel_num;
	stepper->dir_port = dir_port;
	stepper->dir_pin = dir_pin;
	
	pio_set_output(dir_port, dir_pin, HIGH, DISABLE, DISABLE);
	pio_set_peripheral(PIOA, PIO_PERIPH_B, PIO_PA7);

	_init_pwm(stepper);
}

void stepper_set_velocity(stepper_s *stepper, unsigned steps_per_sec, const stepper_dir dir) {
	if (dir == STEPPER_DIR_CW) {
		pio_set(stepper->dir_port, stepper->dir_pin);
	} else {
		pio_clear(stepper->dir_port, stepper->dir_pin);
	}
	
	_pwm_set_freq(stepper, steps_per_sec);
}

void stepper_stop(stepper_s *stepper) {
	stepper_set_velocity(stepper, 0, STEPPER_DIR_CW);
}
