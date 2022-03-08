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

#define RS485_RE				PIO_PA12
#define RS485_RE_PORT			PIOA

#define TEMP_SEL0				PIO_PA8
#define TEMP_SEL0_PORT			PIOA
#define TEMP_SEL1				PIO_PA9
#define TEMP_SEL1_PORT			PIOA
#define TEMP_SEL2				PIO_PA10
#define TEMP_SEL2_PORT			PIOA

#define CELL_SEL0				PIO_PA4
#define CELL_SEL0_PORT			PIOA
#define CELL_SEL1				PIO_PA5
#define CELL_SEL1_PORT			PIOA
#define CELL_SEL2				PIO_PA6
#define CELL_SEL2_PORT			PIOA

#define AFE_EN					PIO_PA7
#define AFE_EN_PORT				PIOA

#define PWR_SW					PIO_PA0
#define PWR_SW_PORT				PIOA

#define NBAT_EN					PIO_PA11
#define NBAT_EN_PORT			PIOA

#define BOARD_LED				PIO_PA3
#define BOARD_LED_PORT			PIOA

#endif // CONF_BOARD_H
