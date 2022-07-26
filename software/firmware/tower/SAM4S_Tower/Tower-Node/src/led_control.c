#include "led_control.h"
#include "FastLED.h"
#include "modbus.h"

#define NUM_PIXELS 60

CRGB leds[NUM_PIXELS];

static void _run_leds(void) {
	
}

static void _rainbow(void) {
	
}

static void _rainbow_with_glitter(void) {
	
}

static void _confetti(void) {
	
}

static void _sinelon(void) {
	
}

static void _bpm(void) {
	
}

static void _juggle(void) {
	
}

static CRGB _status_scale(int scale, int val) {
	
}

static void _handle_lights(int lights) {
	switch (lights) {
	case 0:
		_run_leds();
		break;
	case 1:
		_rainbow();
		break;
	case 2:
		_rainbow_with_glitter();
		break;
	case 3:
		_confetti();
		break;
	case 4:
		_sinelon();
		break;
	case 5:
		_bpm();
		break;
	case 6:
		_juggle();
		break;
	}
}

static void _handle_drive_state(int drive_state) {
	switch(drive_state){
	case 0:
		// leds[59] = blue;
		break;
	case 1:
		// leds[59] = pink;
		break;
	case 2:
		// leds[59] = green;
		break;
	}
}

static void _handle_waypoint_state(int waypoint_state) {
	switch(waypoint_state){
	case 0:
		// leds[57] = red;
		break;
	case 1:
		// leds[57] = yellow;
		break;
	case 2:
		//leds[57] = green;
		break;
	case 3:
		// leds[57] = blue;
		break;
	}
}

void set_LEDs(void) {
	// Get bits 3-4
	int drive_state = (intRegisters[LED_CONTROL_REG] >> 3) & 0x03;
	
	// Get bits 5-6
	int waypoint_state = (intRegisters[LED_CONTROL_REG] >> 5) & 0x03;
	
	// Get bits 7-10
	int lights = (intRegisters[LED_CONTROL_REG] >> 7) & 0x0F;
	
	// Get bits 11-15
	int comms = (intRegisters[LED_CONTROL_REG] >> 11) & 0x1F;
	
	_handle_lights(lights);
	
	if(!communicationGood()) {
		for(int i = 57; i < 60; i++) {
			// leds[i] = purple;
		}
	} else {
		_handle_waypoint_state(waypoint_state);
		leds[58] = _status_scale(32, comms);
		_handle_drive_state(drive_state);
	}
	
	// FastLED.show();
}
