/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H


#define MODBUS_BPS 115200
#define MODBUS_TIMEOUT 20000				//increased to deal with blocking code potentially causing issues.
#define MODBUS_SER_PORT UART0
#define MODBUS_EN_PORT PIOA
#define MODBUS_EN_PIN PIO_PA11

#define STEPPER_PWM_CHANNEL PWM_CHANNEL_3
#define STEPPER_DIR_PORT PIOA
#define STEPPER_DIR_PIN PIO_PA6
#define STEPPER_STEP_PORT PIOA
#define STEPPER_STEP_PIN PIO_PA7


#define LSw1_PORT				PIOA
#define LSw1_PIN				PIO_PA0

#define LSw2_PORT				PIOA
#define LSw2_PIN				PIO_PA1

#define LSw3_PORT				PIOA
#define LSw3_PIN				PIO_PA2

#define LSw4_PORT				PIOA
#define LSw4_PIN				PIO_PA3

#define Lazer_PORT				PIOA
#define Lazer_PIN				PIO_PA13

#define vidSel0_PORT			PIOA
#define vidSel0_PIN				PIO_PA14

#define vidSel1_PORT			PIOA
#define vidSel1_PIN				PIO_PA15

#endif // CONF_BOARD_H
