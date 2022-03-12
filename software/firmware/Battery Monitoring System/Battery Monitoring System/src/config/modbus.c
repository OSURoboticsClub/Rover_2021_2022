/*
 * modbus.c
 *
 * Created: 2/3/2022 3:38:07 PM
 *  Author: Anthony Grana,
			Blake H,
			Kurtis Dinelle
 */ 


#include <asf.h>
#include <modbus.h>
#include <string.h> // For memcpy

Uart *RS485Port;
uint8_t slaveID;

Pio *globalEnPinPort;
uint32_t globalEnPin;


//uint16_t	recievedDataSize = 0;
//uint16_t	transmitDataSize = 0;
//uint8_t		rxBuffer[RX_BUFFER_SIZE];
uint16_t	packetSize;							
uint16_t	transmitIndex;					//helper variables for transmitting
bool		endTransmission;

uint16_t	intRegisters[REGISTER_AR_SIZE];
float		floatRegisters[REGISTER_AR_SIZE];
char		charRegisters[REGISTER_AR_SIZE];
bool		boolRegisters[REGISTER_AR_SIZE];


uint8_t responsePacket[TX_BUFFER_SIZE];
uint16_t responsePacketSize;

struct ringBuffer{																	//ring buffer to serve as rx buffer. Helps solve lots of data shifting problems
	uint16_t head; // Next free space in the buffer
	uint16_t tail; // Start of unprocessed data and beginning of packet
	uint8_t data[RX_BUFFER_SIZE];
} rxBuffer = {
	.head = 0,
	.tail = 0};

// To handle edge case if a packet starts toward end of buffer but wraps around to beginning
#define PKT_WRAP_ARND(idx) \
((idx) & (RX_BUFFER_SIZE - 1))

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

static uint8_t* floatToBytes_union(float f){
	union {
		uint32_t data;
		float data_f;
	} u;

	static uint8_t floatCoversionBytes[FLOAT_REG_BYTE_SZ];

	u.data_f = f;
	floatCoversionBytes[0] = (u.data >> 24) & 0xFF;
	floatCoversionBytes[1] = (u.data >> 16) & 0xFF;
	floatCoversionBytes[2] = (u.data >> 8) & 0xFF;
	floatCoversionBytes[3] = u.data & 0xFF;
	
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


void readHandler(uint8_t* responsePacket, uint16_t start_reg, uint16_t end_reg) {
	int i = start_reg;
	while (i < REGISTER_AR_SIZE+INT_REG_OFFSET && i <= end_reg) {
		uint16_t data = intRegisters[i-INT_REG_OFFSET];
		responsePacket[0] = (data >> 8) & 0xFF;
		responsePacket[1] = data & 0xFF;
		responsePacket += INT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+FLOAT_REG_OFFSET && i <= end_reg) {
		uint8_t* floatConversionBytes = floatToBytes_union(floatRegisters[i-FLOAT_REG_OFFSET]);
		for (int j = 0; j < FLOAT_REG_BYTE_SZ; j++) {
			responsePacket[j] = floatConversionBytes[j];
		}
		responsePacket += FLOAT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+CHAR_REG_OFFSET && i <= end_reg) {
		responsePacket[0] = charRegisters[i-CHAR_REG_OFFSET];
		responsePacket += CHAR_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+BOOL_REG_OFFSET && i <= end_reg) {
		responsePacket[0] = boolRegisters[i-BOOL_REG_OFFSET];
		responsePacket += BOOL_REG_BYTE_SZ;
		i++;
	}
}

void writeHandler(uint8_t* data_packet, uint16_t start_reg, uint16_t end_reg) {
	int i = start_reg;
	while (i < REGISTER_AR_SIZE+INT_REG_OFFSET && i < end_reg) {
		intRegisters[i-INT_REG_OFFSET] = convertToInt(data_packet);
		data_packet += INT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+FLOAT_REG_OFFSET && i < end_reg) {
		floatRegisters[i-FLOAT_REG_OFFSET] = convertToFloat_union(data_packet);
		data_packet += FLOAT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+CHAR_REG_OFFSET && i < end_reg) {
		charRegisters[i-CHAR_REG_OFFSET] = convertToChar(data_packet);
		data_packet += CHAR_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+BOOL_REG_OFFSET && i < end_reg) {
		boolRegisters[i-BOOL_REG_OFFSET] = convertToBool(data_packet);
		data_packet += BOOL_REG_BYTE_SZ;
		i++;
	}
}

uint16_t getReadResponseDataSize(uint16_t start_reg, uint16_t end_reg) {
	uint16_t size = 0;																			//I don't like the previous implementation because it had several loops that were unnecessary

	if(start_reg < REGISTER_AR_SIZE+INT_REG_OFFSET){									//check if starting register is within the data type range
		if(end_reg >= REGISTER_AR_SIZE+INT_REG_OFFSET){									//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+INT_REG_OFFSET-start_reg)*INT_REG_BYTE_SZ;		//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+INT_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += ((end_reg+1) - start_reg)*INT_REG_BYTE_SZ;							//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+FLOAT_REG_OFFSET){									//check if starting register is within the data type range
		if(end_reg >= REGISTER_AR_SIZE+FLOAT_REG_OFFSET){								//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+FLOAT_REG_OFFSET-start_reg)*FLOAT_REG_BYTE_SZ;	//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+FLOAT_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += ((end_reg+1) - start_reg)*FLOAT_REG_BYTE_SZ;						//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+CHAR_REG_OFFSET){									//check if starting register is within the data type range
		if(end_reg >= REGISTER_AR_SIZE+CHAR_REG_OFFSET){								//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+CHAR_REG_OFFSET-start_reg)*CHAR_REG_BYTE_SZ;		//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+CHAR_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += ((end_reg+1) - start_reg)*CHAR_REG_BYTE_SZ;							//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+BOOL_REG_OFFSET){
		size += ((end_reg+1) - start_reg)*BOOL_REG_BYTE_SZ;								//return the size including this data type's registers
		return size;
	}
		
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
	
	uint32_t clockSpeed = sysclk_get_peripheral_bus_hz(RS485Port);		//gets CPU speed to for baud counter
	
	sam_uart_opt_t UARTSettings = {
		.ul_baudrate = baud,			//sets baudrate
		.ul_mode = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO,	//sets to normal mode
		.ul_mck = clockSpeed			//sets baud counter clock
	};
	
	uart_init(RS485Port, &UARTSettings);							//init the UART port
	uart_enable_rx(RS485Port);
	uart_enable_tx(RS485Port);
	uart_enable_interrupt(RS485Port, UART_IER_RXRDY);				//Enable interrupt for incoming data
	
	pio_set_output(enPinPort,enPin,LOW,DISABLE,DISABLE);		//init the enable pin
	globalEnPinPort = enPinPort;
	globalEnPin = enPin;
	
	/*																//CRC engine cannot be used in the current configuration because the modbus RTU polynomial (0xA001) does not match any of the supported polynomials
	uint8_t CRCMode = CRCCU_MR_ENABLE | CRCCU_MR_PTYPE_CCITT16;
	pmc_enable_periph_clk(ID_CRCCU);							//init CRC Computation Unit
	crccu_configure_mode(CRCCU, CRCMode)
	*/
}



void modbus_update(void){
	if(buffer_get_data_sz() < ABS_MIN_PACKET_SIZE) return;			//if not enough data has been received just break out
	if( !packet_complete()) return;									//check if an entire packet has been received otherwise return, also resolves overflow errors
	uint8_t* packet = pop_packet();									//packet is complete, so pull it out
	if(packet[SLAVE_ID_IDX] != slaveID) return;						//disregard if the packet doesn't apply to this slave
	uint8_t CRC[2] = {packet[packetSize-2], packet[packetSize-1]};
	// extract register info from packet
	uint16_t start_reg = packet[START_REG_H_IDX] << 8 | packet[START_REG_L_IDX];
	uint16_t end_reg = packet[END_REG_H_IDX] << 8 | packet[END_REG_L_IDX];
	// call read and write handlers based on function code
	switch(packet[FC_IDX]) {
		case FC_READ_MULT: ;													//semicolon resolves statement errors
			uint16_t read_num_bytes = getReadResponseDataSize(start_reg, end_reg);
			responsePacketSize = RD_RESP_PACKET_MIN_SIZE + read_num_bytes;
			//responsePacket[SLAVE_ID_IDX] = packet[SLAVE_ID_IDX];				//this was how the protocol used to be
			responsePacket[SLAVE_ID_IDX] = MASTER_ADRESS;						//this is how the protocol is now to help identify when the master or slave is speaking
			responsePacket[FC_IDX] = packet[FC_IDX];
			responsePacket[RD_DATA_SIZE_IDX] = read_num_bytes;
			readHandler(responsePacket+RD_DATA_BYTE_START, start_reg, end_reg);
			break;
		case FC_WRITE_MULT: ;
			responsePacketSize = WR_RESP_PACKET_SIZE;
			//responsePacket[SLAVE_ID_IDX] = packet[SLAVE_ID_IDX];
			responsePacket[SLAVE_ID_IDX] = MASTER_ADRESS;	
			responsePacket[FC_IDX] = packet[FC_IDX];
			responsePacket[START_REG_H_IDX] = packet[START_REG_H_IDX];
			responsePacket[START_REG_L_IDX] = packet[START_REG_L_IDX];
			responsePacket[END_REG_H_IDX] = packet[END_REG_H_IDX];
			responsePacket[END_REG_L_IDX] = packet[END_REG_L_IDX];
			writeHandler(&packet[WR_DATA_BYTE_START], start_reg, end_reg);
			break;
	}
	// add the CRC to the response packet here
	uint16_t responceCRC = ModRTU_CRC(responsePacket, responsePacketSize-CRC_SIZE);			//calculate crc
	
	responsePacket[responsePacketSize-2] = responceCRC & 0xff;								//add CRC
	responsePacket[responsePacketSize-1] = (responceCRC>>8) & 0xff;
	
	// write out response packet
	pio_set(globalEnPinPort,globalEnPin);				//transceiver transmit enable
	pio_set(RS485_NRE_PORT,RS485_NRE);
	transmitIndex = 0;
	uart_enable_interrupt(RS485Port,UART_IMR_TXRDY);
}

//interrupt handler for incoming data
void UART_Handler(void){
	if(uart_is_rx_ready(RS485Port)){							//confirm there is data ready to be read
		uart_read(RS485Port, &(rxBuffer.data[rxBuffer.head]));		//move the data into the next index of the rx buffer
		rxBuffer.head = PKT_WRAP_ARND(rxBuffer.head + 1);		//iterate the head through the ring buffer
	}else if(uart_is_tx_ready(RS485Port)){
		if(transmitIndex < responsePacketSize){
			uart_write(RS485Port, responsePacket[transmitIndex]);
			transmitIndex++;
		}else if(uart_is_tx_empty(RS485Port)){
			pio_clear(globalEnPinPort,globalEnPin);
			pio_clear(RS485_NRE_PORT,RS485_NRE);
			uart_disable_interrupt(RS485Port,UART_IMR_TXRDY);
		}
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
	static uint8_t returnPacket[RX_BUFFER_SIZE];
	for(int i=0;i<packetSize;i++){							//copy packet data to return array
		returnPacket[i] = rxBuffer.data[rxBuffer.tail];
		rxBuffer.tail = PKT_WRAP_ARND(rxBuffer.tail + 1);	//iterate the tail
	}
	return returnPacket;									//return
}

uint16_t buffer_get_data_sz(void) {
	if (rxBuffer.head >= rxBuffer.tail) {
		return rxBuffer.head - rxBuffer.tail;
	} else {
		return (RX_BUFFER_SIZE - rxBuffer.tail) + rxBuffer.head;
	}
}

bool packet_complete(){
	packetSize = 0;																	// Reset this in case packet is not complete
	
	uint8_t func_code = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + FC_IDX)];
	
	if((func_code != FC_WRITE_MULT) && (func_code != FC_READ_MULT)){				//if the function code isn't write or read, we know somethings fucked up
		//need to write a graceful handler here
		//needs to be a while loop that iterates through the buffer looking for a valid function code, then pops out all the garbage that came before 
		uint8_t checkByte = func_code;
		uint16_t FCLoc = PKT_WRAP_ARND(rxBuffer.tail + FC_IDX);
		while(checkByte != FC_READ_MULT && checkByte != FC_WRITE_MULT && FCLoc != rxBuffer.head){
			FCLoc = PKT_WRAP_ARND(FCLoc + 1);
			checkByte = rxBuffer.data[FCLoc];
		}
		if(PKT_WRAP_ARND(FCLoc-1) >= rxBuffer.tail){
			packetSize = PKT_WRAP_ARND(FCLoc-1) - rxBuffer.tail;
		}else{
			packetSize = (RX_BUFFER_SIZE - rxBuffer.tail) + PKT_WRAP_ARND(FCLoc-1);
		}
		pop_packet();
		return false;																
	}
	uint8_t slave_id = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + SLAVE_ID_IDX)];
	
	//bool need_crc = false;															//helper variables				//need crc becomes redundent, because at a certain point you need the crc no mater what
	uint8_t num_data_bytes = 0;														// Default 0 for packets with no data bytes
	uint16_t base_pkt_sz;															//size of packet not including data bytes
	
	// Handle write command from master
	if (func_code == FC_WRITE_MULT && slave_id != 0) {
		if(buffer_get_data_sz() < ABS_MIN_WRITE_PACKET_SIZE) return false;						//if the data size is less than this, we know the packet is incomplete
		num_data_bytes = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + WR_DATA_SIZE_IDX)];		//get supposed number of data bytes from the packet
		base_pkt_sz = ABS_MIN_WRITE_PACKET_SIZE - 1;											
		//need_crc = true;																		
	}
	
	// Handle write response from slave or read command from master (identical packet structure)
	else if ((func_code == FC_WRITE_MULT && slave_id == 0) ||
	((func_code == FC_READ_MULT) && slave_id != 0)) {
		if(buffer_get_data_sz() < WRITE_RES_PACKET_SIZE) return false;					//if the data size is less than this, we know the packet is incomplete
		base_pkt_sz = WRITE_RES_PACKET_SIZE;											//we know the final packet size
		//need_crc = true;																//still need crc because we need to confirm we are processing a complete packet and not some segment of another packet
	}
	
	// Handle read response from slave
	else if (func_code == FC_READ_MULT && slave_id == 0) {
		//if(buffer_get_data_sz() < ABS_MIN_READ_RES_PACKET_SIZE) return false;			//if the data size is less than this, we know the packet is incomplete     //Redundent due to check before entering the function
		num_data_bytes = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + RD_DATA_SIZE_IDX)];
		base_pkt_sz = ABS_MIN_READ_RES_PACKET_SIZE - 1;
		//need_crc = true;
	}
	
	uint16_t full_pkt_sz = num_data_bytes + base_pkt_sz;								//calculate full packet size
	
	if (buffer_get_data_sz() < full_pkt_sz) return false;								//make sure we have a full packet
	
	packetSize = full_pkt_sz;															// Set global packetSize to completed packet size
	
	uint8_t packetNoCRC[packetSize - CRC_SIZE];											//pull packet into linear buffer for crc check
	for(int i=0;i<packetSize - CRC_SIZE;i++){
		packetNoCRC[i] = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + i)];
	}
	
	uint8_t packetCRC[CRC_SIZE];														//pull out the crc from the packet
	for(int i=0; i < CRC_SIZE; i++){
		packetCRC[i] = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + (packetSize - CRC_SIZE) + i)];
	}
	
	uint16_t expectedCRC = ModRTU_CRC(packetNoCRC, packetSize - CRC_SIZE);				//calculate expected crc
	
	if(((expectedCRC >> 8) & 0xFF) == packetCRC[1] && (expectedCRC & 0xFF) == packetCRC[0]){				//crc comparison
		return true;																	//packet is complete and passes crc
	}else{																				//on crc fail remove first byte from buffer This is the only known incorrect byte
		packetSize = 1;
		pop_packet();
		return false;
	}
	
}

uint16_t ModRTU_CRC(uint8_t* buf, int len)
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
	
	return crc;
}