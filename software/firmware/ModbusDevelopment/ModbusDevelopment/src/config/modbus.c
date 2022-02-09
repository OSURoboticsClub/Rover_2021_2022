/*
 * CFile1.c
 *
 * Created: 2/3/2022 3:38:07 PM
 *  Author: Anthony
 */ 


#include <asf.h>
#include <modbus.h>

Uart *RS485Port;
uint8_t slaveID;

uint16_t		recievedDataSize = 0;
uint16_t		transmitDataSize = 0;
const uint8_t	rxBuffer[RX_BUFFER_SIZE];
const uint8_t	txBuffer[TX_BUFFER_SIZE];

const uint16_t	intRegisters[REGISTER_AR_SIZE];
const float		floatRegisters[REGISTER_AR_SIZE];
const char		charRegisters[REGISTER_AR_SIZE];
const bool		boolRegisters[REGISTER_AR_SIZE];






void modbus_init(Uart *port485, const uint32_t baud, Pio *enPinPort, const uint32_t enPin, const uint8_t slave_id){
	
	RS485Port = port485;
	slaveID = slave_id;
	
	if(RS485Port == UART0){
		pmc_enable_periph_clk(ID_UART0);		//Enable the clocks to the UART modules
		pio_set_peripheral(PIOA,PIO_PERIPH_A,PIO_PA9);		//Sets PA9 to RX
		pio_set_peripheral(PIOA,PIO_PERIPH_A,PIO_PA10);		//Sets PA10 to TX
		NVIC_EnableIRQ(UART0_IRQn);							//enables interrupts related to this port
	}
	
	if(RS485Port == UART1){
		pmc_enable_periph_clk(ID_UART1);		//Enable the clocks to the UART modules
		pio_set_peripheral(PIOB,PIO_PERIPH_A,PIO_PB2);		//Sets PB2 to RX
		pio_set_peripheral(PIOB,PIO_PERIPH_A,PIO_PB3);		//Sets PB3 to TX
		NVIC_EnableIRQ(UART1_IRQn);							//enables interrupts related to this port
	}
	
	uint32_t clockSpeed = sysclk_get_cpu_hz();		//gets CPU speed to for baud counter
	
	sam_uart_opt_t UARTSettings = {
		.ul_baudrate = baud,			//sets baudrate
		.ul_mode = 0,					//sets to normal mode
		.ul_mck = clockSpeed			//sets baud counter clock
	};
	
	uart_init(RS485Port, &UARTSettings);							//init the UART port
	uart_enable_interrupt(RS485Port, UART_IER_RXRDY);				//Enable interrupt for incoming data
	
	pio_set_output(enPinPort,enPin,HIGH,DISABLE,DISABLE);		//init the enable pin
}




void modbus_update(void){
	if(recievedDataSize < ABS_MIN_PACKET_SIZE) return;			//if not enough data has been received just break out
	if(! packet_complete()) return;								//check if an entire packet has been received otherwise return
	uint8_t* packet = pop_packet();								//packet is complete, so pull it out
	if(packet[SLAVE_ID_IDX] != slaveID) return;					//disregard if the packet doesn't apply to this slave
}

//interrupt handler for incoming data
void UART_Handler(void){
	if(uart_is_tx_ready(RS485Port)){							//confirm there is data ready to be read
		uart_read(RS485Port, rxBuffer[recievedDataSize]);		//move the data into the next index of the rx buffer
		recievedDataSize = recievedDataSize < RX_BUFFER_SIZE ? recievedDataSize + 1 : recievedDataSize;  //iterate receive buffer size if not yet full
	}
}


//Regardless of what UART port triggers the interrupt, the behavior is the same
void UART0_Handler(){
	UART_Handler();
}

void UART1_Handler(){
	UART_Handler();
}

uint8_t* pop_packet(){
	
}

bool packet_complete(){

}

uint8_t* ModRTU_CRC(uint8_t* buf, int len)
{
	uint8_t crc = 0xFFFF;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint8_t)buf[pos];          // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
			crc >>= 1;                    // Just shift right
		}
	}
	uint8_t crcBytes[2] = {0x00,0x00};
	crcBytes[0] |= crc;
	crcBytes[1] |= crc>>8;
	return crcBytes;
}