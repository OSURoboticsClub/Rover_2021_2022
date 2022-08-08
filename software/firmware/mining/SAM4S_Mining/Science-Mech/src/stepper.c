#include "stepper.h"
#include <asf.h>

/* The higher this is, the more accurate the PWM.
 * However, the higher it is means the lower max frequency supported due to overflow.
 */

#define PWM_CLK_SPEED 10000

void setPosition(stepper_s *stepper, int pos){
	int delta = pos - stepper->position;			//find out how we need to move
	
	if(delta > 0) pio_set(stepper->dir_port, stepper->dir_pin);		//sets direction positive or negative.
	else pio_clear(stepper->dir_port, stepper->dir_pin);
	
	delta = delta > 0? delta : -delta;				//take absolute value of delta
	
	pwm_channel_t pwm_channel_instance = {			// !! I'm assuming all of the PWM stuff you did before is good
		.ul_prescaler = PWM_CMR_CPRE_CLKA,			//I would double check that the PWM signal is actually left aligned.
		.ul_period = 100,
		.ul_duty = 50,																//Sets pulse width to 5 ms, (i micro second min)
		.channel = stepper->PWM_Channel
	};
	pwm_channel_disable(PWM, stepper->PWM_Channel);
	pwm_channel_init(PWM, &stepper->PWM_Channel);
	pwm_channel_enable(PWM, stepper->PWM_Channel);
	
	for (int i=0; i<delta; i++){					//make a step for each step that needs to be made. then disable PWM
		while(!PWM->PWM_ISR1 | PWM_ISR1_CHID3){}	//Wait until counter event occurs (on counter reset) Reading this register clears it, !! Double check that the register is actually clearing. Reference page 961 of the datasheet
	}												//I chose the blocking method because dealing with interrupts without testing is hard.
	pwm_channel_disable(PWM, stepper->PWM_Channel);
	
	stepper->position = pos;
}

int getPosition(stepper_s *stepper){
	return stepper->position;
}

void stepper_setup(stepper_s *stepper, int micro, Pio *dir_port_i, uint32_t dir_pin_i, Pio *step_port_i, uint32_t step_pin_i, int PWM_Channel_i){
	stepper -> position = 0;
	stepper -> microstepping = micro;
	stepper -> dir_port = dir_port_i;
	stepper -> dir_pin = dir_pin_i;
	stepper -> step_port = step_port_i;
	stepper -> step_pin = step_pin_i;
	stepper -> PWM_Channel = PWM_Channel_i;
	
	pio_set_output(dir_port_i, dir_pin_i, HIGH, DISABLE, DISABLE);
	
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, stepper->PWM_Channel);
	
	pwm_clock_t clock_setting = {
		.ul_clka = PWM_CLK_SPEED,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_peripheral_bus_hz(PWM)
	};
	pwm_init(PWM, &clock_setting);
}

/* Old implentation.
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
	int period = PWM_CLK_SPEED / (freq <= PWM_CLK_SPEED ? freq : PWM_CLK_SPEED);		//you wind up dividing by zero here when you set speed to zero. ( will change the implimentation)
	
	pwm_channel_t pwm_channel_instance = {
		.ul_prescaler = PWM_CMR_CPRE_CLKA,
		.ul_period = period,
		.ul_duty = 50,																//Sets pulse width to 1 ms, (This is pretty standard, and the chip says is supports down to 1 micro second pulse)
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
	pwm_channel_disable(PWM, stepper->pwm_channel_num);					//New implementation just disables PWM, instead of setting "infinite" -> (unkown) period
}
*/