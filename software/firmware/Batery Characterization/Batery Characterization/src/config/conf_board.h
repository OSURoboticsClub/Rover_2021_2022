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

#define BUILT_IN_LED PIO_PA8
#define MUX_EN PIO_PA7
#define MUX_A0 PIO_PA6
#define MUX_A1 PIO_PA5
#define MUX_A2 PIO_PA4
#define FET_PWM PIO_PA0
#define BOARD_TEMP1 ADC_CHANNEL_4
#define BOARD_TEMP2 ADC_CHANNEL_5
#define BOARD_CELLV ADC_CHANNEL_0
#define BOARD_SHUNT ADC_CHANNEL_1
#define THERMISTOR_BETA 4700
#define THERMISTOR_BETA_TEMP 25

/** Reference voltage for ADC,in mv. */
#define VOLT_REF        (3300)
/** The maximal ADC digital value */
#define MAX_DIGITAL     (4095)
/* ADC Tracking Time*/
#define TRACKING_TIME    1
/* ADC Transfer Period */
#define TRANSFER_PERIOD  1

//USB see conf_usb.h

//Timer Counter
#define TC TC0
#define TC_CH 1
#define TransmitTimerHandler TC1_Handler

//PWM TC
/*
#define PWM TC0
#define PWM_CH 0
*/
#define LOAD_PORT PIOA
#define LOAD_MASK PIO_PA0
#define LOAD_PERIPH PIO_PERIPH_A

#endif // CONF_BOARD_H
