/*
 * CFile1.c
 *
 * Created: 2/3/2022 3:38:07 PM
 *  Author: Anthony
 */ 


#include <asf.h>
#include <modbus.h>
#include <string.h> // For memcpy

Uart *RS485Port;
uint8_t slaveID;

uint16_t		recievedDataSize = 0;
uint16_t		transmitDataSize = 0;
const uint8_t	rxBuffer[RX_BUFFER_SIZE];
const uint8_t	txBuffer[TX_BUFFER_SIZE];

uint16_t	intRegisters[REGISTER_AR_SIZE];
float		floatRegisters[REGISTER_AR_SIZE];
char		charRegisters[REGISTER_AR_SIZE];
bool		boolRegisters[REGISTER_AR_SIZE];



/* All convert functions assume they are receiving unsigned values in
 * big-endian format. */

#define MERGE_FOUR_BYTES(x) \
    (((x)[0] << 24) | ((x)[1] << 16) | ((x)[2] << 8) | (x)[3])

// Three different methods for converting uint to float
static float convertToFloat_union(const uint8_t data[4]){
    union {
        uint32_t data;
        float data_f;
    } u;

    u.data = MERGE_FOUR_BYTES(data);
    return u.data_f;
}

uint8_t floatCoversionBytes[FLOAT_REG_BYTE_SZ];

static uint8_t* floatToBytes_union(float f){
	union {
		uint32_t data;
		float data_f;
	} u;

	u.data_f = f;
	floatCoversionBytes[0] = (u.data >> 24) & 0xFFFF;
	floatCoversionBytes[1] = (u.data >> 16) & 0xFFFF;
	floatCoversionBytes[2] = (u.data >> 8) & 0xFFFF;
	floatCoversionBytes[3] = u.data & 0xFFFF};
	return floatCoversionBytes;
}

static float convertToFloat_memcpy(const uint8_t data[4]){
    uint32_t merged = MERGE_FOUR_BYTES(data);
    float f = 0.0f;

    // Should not cause buffer overflow if float is guaranteed size 4
    memcpy(&f, &merged, 4);
    return f;
}


static float convertToFloat_recast(const uint8_t data[4]){
    /* We can't just cast the data pointer because if bytes are received in
     * big endian and we are on a little endian machine (or vice-versa),
     * we get a garbage value. So we just merge them into a single uint32. */
    uint32_t merged = MERGE_FOUR_BYTES(data);
    return *((float *)&merged);
}

// These are so trivial they probably don't even need to be functions
static uint16_t convertToInt(const uint8_t data[2]){
    return (data[0] << 8) | data[1];
}

static char convertToChar(const uint8_t data[1]){
    return data[0];
}

static bool convertToBool(const uint8_t data[1]){
    return data[0];
}


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
	size_t packet_len = sizeof(packet);
	uint8_t CRC[2] = {packet[packet_len-2], packet[packet_len-1]};
	// check CRC here?
	// extract register info from packet
	uint16_t start_reg = packet[START_REG_H_IDX] << 8 | packet[START_REG_L_IDX];
	uint16_t end_reg = packet[END_REG_H_IDX] << 8 | packet[END_REG_L_IDX];
	uint8_t num_bytes;
	uint8_t* responsePacket;
	// call read and write handlers based on function code
	switch(packet[FC_IDX]) {
		case FC_READ_MULT:
			responsePacket = readHandler(start_reg, end_reg);
			break;
		case FC_WRITE_MULT:
			responsePacket = writeHandler(&packet[WR_DATA_BYTE_START], start_reg, end_reg);
			break;
	}
	// write out response packet
	uint16_t responsePacketSize = sizeof(responsePacket);
	for (int i = 0; i < responsePacketSize; i++) {
		uart_write(RS485Port, responsePacket[i]);
	}
}

//interrupt handler for incoming data
void UART_Handler(void){
	if(uart_is_tx_ready(RS485Port)){							//confirm there is data ready to be read
		uart_read(RS485Port, rxBuffer[recievedDataSize]);		//move the data into the next index of the rx buffer
		recievedDataSize = recievedDataSize < RX_BUFFER_SIZE ? recievedDataSize + 1 : 0;  //iterate receive buffer size if not yet full
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
	uint8_t returnPacket[packetSize];						//the popped packet array
	for(int i=0;i<packetSize;i++){							//copy packet data to packet array
		returnPacket[i] = rxBuffer[i];
	}
	for(int i=0;i<recievedDataSize-packetSize;i++){			//shift rx buffer to the left
		rxBuffer[i] = rxBuffer[i+packetSize];
	}
	recievedDataSize -= packetSize;							//decrease array size by the length of the 
	for(int i=0;i<packetSize;i++){							//assert 0s to the end of the rx buffer
		rxBuffer[recievedDataSize+i] = 0;
	}
	return returnPacket;									//return the packet
}

bool packet_complete(){

}

uint8_t* ModRTU_CRC(uint8_t* buf, int len)
{
	uint16_t crc = 0xFFFF;

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