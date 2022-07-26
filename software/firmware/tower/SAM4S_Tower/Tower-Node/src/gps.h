/* This is a GPS library specifically for the tower-node board of the Mars Rover.
 * The GPS module is connected to a USART port (plain UART is not available for these pins).
 * Received data from the GPS module is formatted as JSON and then relayed over a different UART port.
*/

#ifndef GPS_H
#define GPS_H

// Configuration for port that GPS module is connected to
#define GPS_IN_SER_PORT		USART0
#define GPS_IN_CLK			ID_USART0
#define GPS_IN_ISR			USART0_IRQn // Make sure to change interrupt handler name if this changes
#define GPS_IN_GPIO_PORT	PIOA
#define GPS_IN_RX_PIN		PIO_PA5
#define GPS_IN_BPS			9600 // Likely needs to be kept at 9600

// Configuration for port that GPS data is relayed over
#define GPS_OUT_SER_PORT	UART1
#define GPS_OUT_CLK			ID_UART1
#define GPS_OUT_ISR			UART1_IRQn // Make sure to change interrupt handler name if this changes
#define GPS_OUT_GPIO_PORT	PIOB
#define GPS_OUT_EN_PIN		PIO_PB1
#define GPS_OUT_TX_PIN		PIO_PB3
#define GPS_OUT_BPS			19200 // 115200 currently not working

#define GPS_BUF_SZ			4096 // Must be power of 2

// Quick way to format GPS data as JSON for transmitting
#define GPS_JSON_START		"{\"gps\":\""
#define GPS_JSON_END		"\"}"
#define GPS_END_OF_PACKET	'\r'
#define GPS_END_OF_TX		GPS_JSON_END "\n\n\r" GPS_JSON_START

void gps_setup(void);
void gps_handle(void);


#endif
