#include <asf.h>
#include "modbus.h"
#include "gps.h"
#include "led_control.h"

// Modbus specific information
#define MODBUS_SLAVE_ID 1
#define MODBUS_BPS 115200
#define MODBUS_TIMEOUT 2000
#define MODBUS_SER_PORT UART0
#define MODBUS_EN_PORT PIOA
#define MODBUS_EN_PIN PIO_PA8


int main(void) {
	sysclk_init();
	board_init();
	
	portSetup(MODBUS_SER_PORT, MODBUS_BPS, MODBUS_EN_PORT, MODBUS_EN_PIN, MODBUS_TIMEOUT);
	modbus_init(MODBUS_SLAVE_ID);
	
	gps_setup();

	while (1) {
		modbus_update();
		set_LEDs();
		gps_handle();
	}
}
