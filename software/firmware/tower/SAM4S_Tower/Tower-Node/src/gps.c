#include <stdbool.h>
#include <string.h>
#include <asf.h>
#include "gps.h"

// Holds received GPS data (circular buffer to minimize data loss)
struct gps_buffer_s {
	char data[GPS_BUF_SZ];
	int tail;
	int head;
} gps_buffer;

// Keeps track of completed packets not yet processed and if a packet is currently being relayed
static int num_packets = 0;
static bool is_tx = false;

// Reads a character from the tail of the buffer and increments tail
static char _gps_buffer_getc(void) {
	char tmp = gps_buffer.data[gps_buffer.tail++];
	gps_buffer.tail &= (GPS_BUF_SZ - 1);
	return tmp;
}

// Adds a character to the head of the buffer and increments head
static void _gps_buffer_putc(char c) {
	gps_buffer.data[gps_buffer.head++] = c;
	gps_buffer.head &= (GPS_BUF_SZ - 1);
}

// Adds a string to the head of the buffer and increments head by length of the string
static void _gps_buffer_puts(const char *data) {
	int data_len = strlen(data);
	
	for (int i = 0; i < data_len; i++) {
		_gps_buffer_putc(data[i]);
	}
}

// Setup the serial port for incoming GPS data
static void _gps_in_setup(void) {
	pmc_enable_periph_clk(GPS_IN_CLK);
	pio_set_peripheral(GPS_IN_GPIO_PORT, PIO_PERIPH_A, GPS_IN_RX_PIN);
	NVIC_EnableIRQ(GPS_IN_ISR);
	
	uint32_t clockSpeed = sysclk_get_peripheral_bus_hz(GPS_IN_SER_PORT);

	sam_usart_opt_t USARTSettings = {
		.channel_mode = US_MR_CHMODE_NORMAL,
		.char_length = US_MR_CHRL_8_BIT,
		.parity_type =  US_MR_PAR_NO,
		.stop_bits = US_MR_NBSTOP_1_BIT,
		.baudrate = GPS_IN_BPS
	};

	usart_init_rs232(GPS_IN_SER_PORT, &USARTSettings, clockSpeed);
	usart_enable_rx(GPS_IN_SER_PORT);
	usart_enable_interrupt(GPS_IN_SER_PORT, US_IER_RXRDY);
}

// Setup the serial port for outgoing GPS data
static void _gps_out_setup(void) {
	pmc_enable_periph_clk(GPS_OUT_CLK);
	pio_set_peripheral(GPS_OUT_GPIO_PORT, PIO_PERIPH_A, GPS_OUT_TX_PIN);
	NVIC_EnableIRQ(GPS_OUT_ISR);
		
	uint32_t clockSpeed = sysclk_get_peripheral_bus_hz(GPS_OUT_SER_PORT);

	sam_uart_opt_t UARTSettings = {
		.ul_baudrate = GPS_OUT_BPS,
		.ul_mode = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO,
		.ul_mck = clockSpeed
	};

	uart_init(GPS_OUT_SER_PORT, &UARTSettings);
	uart_enable_tx(GPS_OUT_SER_PORT);

	pio_set_output(GPS_OUT_GPIO_PORT, GPS_OUT_EN_PIN, LOW, DISABLE, DISABLE);
}

// Initializes GPS library
void gps_setup(void) {
	_gps_in_setup();
	_gps_out_setup();
	
	// Start the buffer with the beginning of JSON formatting
	_gps_buffer_puts(GPS_JSON_START);
}

// Transmits completed GPS packets if any
void gps_handle(void) {
	if (!is_tx && num_packets > 0) {
		is_tx = true;
		pio_set(GPS_OUT_GPIO_PORT, GPS_OUT_EN_PIN);
		uart_enable_interrupt(GPS_OUT_SER_PORT, UART_IMR_TXRDY);
	}
}

void USART0_Handler(void) {
	if (usart_is_rx_ready(GPS_IN_SER_PORT)) {
		char read_char;
		usart_read(GPS_IN_SER_PORT, &read_char);
		
		if (read_char == GPS_END_OF_PACKET) {
			num_packets++;
			
			// Adds some newlines and the start of the JSON format for next received packet
			_gps_buffer_puts(GPS_END_OF_TX);
		} else if (read_char != '\n') {
			_gps_buffer_putc(read_char);
		}
	}
}

void UART1_Handler(void) {
	if (uart_is_tx_ready(GPS_OUT_SER_PORT)) {
		char write_char = _gps_buffer_getc();

		if (write_char != GPS_END_OF_PACKET) {
			uart_write(GPS_OUT_SER_PORT, write_char);
		} else if (uart_is_tx_empty(GPS_OUT_SER_PORT)) {
			pio_clear(GPS_OUT_GPIO_PORT, GPS_OUT_EN_PIN);
			uart_disable_interrupt(GPS_OUT_SER_PORT, UART_IMR_TXRDY);
			
			// Packet has completed transmitting, so decrement the number of unsent packets in buffer
			num_packets--;
			is_tx = false;
		}
	}
}
