/*
 * modbus.c
 *
 * Created: 2/3/2022 3:38:07 PM
 *  Author: Anthony Grana,
			Blake Hakkila,
			Kurtis Dinelle
 */ 


#include <stdbool.h>
#include <modbus.h>
#include <string.h> // For memcpy

uint8_t slaveID;



//uint16_t	recievedDataSize = 0;
//uint16_t	transmitDataSize = 0;
//uint8_t		rxBuffer[RX_BUFFER_SIZE];
uint16_t	packetSize;							
bool		endTransmission;

uint16_t	intRegisters[REGISTER_AR_SIZE];
float		floatRegisters[REGISTER_AR_SIZE];
char		charRegisters[REGISTER_AR_SIZE];
bool		boolRegisters[REGISTER_AR_SIZE];


uint8_t responsePacket[TX_BUFFER_SIZE];
uint16_t responsePacketSize;

struct ringBuffer rxBuffer = {
	.head = 0,
	.tail = 0};

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

void popToFc()
{
	uint16_t FCLoc = PKT_WRAP_ARND(rxBuffer.tail + FC_IDX + 1);
	// uint16_t startHiLoc = PKT_WRAP_ARND(rxBuffer.tail + START_REG_H_IDX + 1);
	// uint16_t numHiLoc = PKT_WRAP_ARND(rxBuffer.tail + NUM_REG_H_IDX + 1);
	uint8_t checkFCByte = rxBuffer.data[FCLoc];
	// uint8_t checkStartHiByte = rxBuffer.data[startHiLoc];
	// uint8_t checkNumHiByte = rxBuffer.data[numHiLoc];
	// while (((checkFCByte != FC_READ_MULT && checkFCByte != FC_WRITE_MULT) || checkNumHiByte >= 4 || checkStartHiByte >= 4) && FCLoc != rxBuffer.head)
	while (checkFCByte != FC_READ_MULT && checkFCByte != FC_WRITE_MULT && FCLoc != rxBuffer.head)
	{
		FCLoc = PKT_WRAP_ARND(FCLoc + 1);
		// startHiLoc = PKT_WRAP_ARND(startHiLoc + 1);
		// numHiLoc = PKT_WRAP_ARND(numHiLoc + 1);
		checkFCByte = rxBuffer.data[FCLoc];
		// checkStartHiByte = rxBuffer.data[startHiLoc];
		// checkNumHiByte = rxBuffer.data[numHiLoc];
	}
	if (PKT_WRAP_ARND(FCLoc - 1) >= rxBuffer.tail)
	{
		packetSize = PKT_WRAP_ARND(FCLoc - 1) - rxBuffer.tail;
	}
	else
	{
		packetSize = (RX_BUFFER_SIZE - rxBuffer.tail) + PKT_WRAP_ARND(FCLoc - 1);
	}

	pop_packet();
}

// modbus functions
void readHandler(uint8_t* responsePacket, uint16_t start_reg, uint16_t end_reg) {
	int i = start_reg;
	while (i < REGISTER_AR_SIZE+INT_REG_OFFSET && i < end_reg) {
		uint16_t data = intRegisters[i-INT_REG_OFFSET];
		responsePacket[0] = (data >> 8) & 0xFF;
		responsePacket[1] = data & 0xFF;
		responsePacket += INT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+FLOAT_REG_OFFSET && i < end_reg) {
		uint8_t* floatConversionBytes = floatToBytes_union(floatRegisters[i-FLOAT_REG_OFFSET]);
		for (int j = 0; j < FLOAT_REG_BYTE_SZ; j++) {
			responsePacket[j] = floatConversionBytes[j];
		}
		responsePacket += FLOAT_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+CHAR_REG_OFFSET && i < end_reg) {
		responsePacket[0] = charRegisters[i-CHAR_REG_OFFSET];
		responsePacket += CHAR_REG_BYTE_SZ;
		i++;
	}
	while (i < REGISTER_AR_SIZE+BOOL_REG_OFFSET && i < end_reg) {
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
		if(end_reg > REGISTER_AR_SIZE+INT_REG_OFFSET){									//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+INT_REG_OFFSET-start_reg)*INT_REG_BYTE_SZ;		//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+INT_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += (end_reg - start_reg)*INT_REG_BYTE_SZ;							//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+FLOAT_REG_OFFSET){									//check if starting register is within the data type range
		if(end_reg > REGISTER_AR_SIZE+FLOAT_REG_OFFSET){								//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+FLOAT_REG_OFFSET-start_reg)*FLOAT_REG_BYTE_SZ;	//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+FLOAT_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += (end_reg - start_reg)*FLOAT_REG_BYTE_SZ;						//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+CHAR_REG_OFFSET){									//check if starting register is within the data type range
		if(end_reg > REGISTER_AR_SIZE+CHAR_REG_OFFSET){								//check if the ending register is past the data type range
			size += (REGISTER_AR_SIZE+CHAR_REG_OFFSET-start_reg)*CHAR_REG_BYTE_SZ;		//add the register size to the size variable
			start_reg = REGISTER_AR_SIZE+CHAR_REG_OFFSET;								//set the new start range to the first float register
		}else{
			size += (end_reg - start_reg)*CHAR_REG_BYTE_SZ;							//return the size including this data type's registers
			return size;
		}
	}
	
	if(start_reg < REGISTER_AR_SIZE+BOOL_REG_OFFSET){
		size += (end_reg - start_reg)*BOOL_REG_BYTE_SZ;								//return the size including this data type's registers
		return size;
	}
		
	return size;	
}

void modbus_init(const uint8_t slave_id){
	slaveID = slave_id;
}



void modbus_update(void){
	if(buffer_get_data_sz() < ABS_MIN_PACKET_SIZE) return;			//if not enough data has been received just break out
	if( !packet_complete()) return;									//check if an entire packet has been received otherwise return, also resolves overflow errors
	uint8_t* packet = pop_packet();									//packet is complete, so pull it out
	if(packet[SLAVE_ID_IDX] != slaveID) return;						//disregard if the packet doesn't apply to this slave
	// extract register info from packet
	uint16_t start_reg = packet[START_REG_H_IDX] << 8 | packet[START_REG_L_IDX];
	uint16_t num_registers = packet[NUM_REG_H_IDX] << 8 | packet[NUM_REG_L_IDX];
	int end_reg = start_reg + num_registers;                                    // this register number is exclusive, so all valid register numbers are less than end_reg
	// call read and write handlers based on function code
	switch(packet[FC_IDX]) {
		case FC_READ_MULT:
		{
			uint16_t read_num_bytes = getReadResponseDataSize(start_reg, end_reg);
			responsePacketSize = RD_RESP_PACKET_MIN_SIZE + read_num_bytes;
			//responsePacket[SLAVE_ID_IDX] = packet[SLAVE_ID_IDX];				//this was how the protocol used to be
			responsePacket[SLAVE_ID_IDX] = MASTER_ADRESS;						//this is how the protocol is now to help identify when the master or slave is speaking
			responsePacket[FC_IDX] = packet[FC_IDX];
			responsePacket[RD_DATA_SIZE_IDX] = read_num_bytes;
			readHandler(responsePacket+RD_DATA_BYTE_START, start_reg, end_reg);
			break;
		}
		case FC_WRITE_MULT:
		{
			responsePacketSize = WR_RESP_PACKET_SIZE;
			//responsePacket[SLAVE_ID_IDX] = packet[SLAVE_ID_IDX];
			responsePacket[SLAVE_ID_IDX] = MASTER_ADRESS;	
			responsePacket[FC_IDX] = packet[FC_IDX];
			responsePacket[START_REG_H_IDX] = packet[START_REG_H_IDX];
			responsePacket[START_REG_L_IDX] = packet[START_REG_L_IDX];
			responsePacket[NUM_REG_H_IDX] = packet[NUM_REG_H_IDX];
			responsePacket[NUM_REG_L_IDX] = packet[NUM_REG_L_IDX];
			writeHandler(&packet[WR_DATA_BYTE_START], start_reg, end_reg);
			break;
		}
	}
	// add the CRC to the response packet here
	uint16_t responceCRC = ModRTU_CRC(responsePacket, responsePacketSize-CRC_SIZE);			//calculate crc
	
	responsePacket[responsePacketSize-2] = responceCRC & 0xff;								//add CRC
	responsePacket[responsePacketSize-1] = (responceCRC>>8) & 0xff;
	
	// write out response packet
	portWrite(responsePacket, responsePacketSize);
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
	
	uint8_t slave_id = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + SLAVE_ID_IDX)];
	uint8_t func_code = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + FC_IDX)];
	uint8_t start_reg_hi = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + START_REG_H_IDX)];
	uint8_t start_reg_lo = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + START_REG_L_IDX)];
	uint8_t num_reg_hi = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + NUM_REG_H_IDX)];
	uint8_t num_reg_lo = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + NUM_REG_L_IDX)];
	uint8_t num_data_bytes = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + WR_DATA_SIZE_IDX)];

	// if the function code isn't write or read or start register/number of registers high bytes are too big, we know somethings fucked up
	// also we may as well skip if the slave id doesn't match
	if ((func_code != FC_WRITE_MULT && func_code != FC_READ_MULT) || start_reg_hi >= 4 || num_reg_hi >= 4 || slave_id != slaveID)
	{
		// need to write a graceful handler here
		// needs to be a while loop that iterates through the buffer looking for a valid function code, then pops out all the garbage that came before
		popToFc();
		return false;
	}

	// lets make sure the number of data bytes is correct
	if (func_code == 0x10 && num_data_bytes != getReadResponseDataSize(start_reg_hi << 8 & start_reg_lo, num_reg_hi << 8 & num_reg_lo)) // calc what size should be for write packet here????????????????????????????????????????????????
	{
		popToFc();
		return false;
	}
	
	//bool need_crc = false;															//helper variables				//need crc becomes redundent, because at a certain point you need the crc no mater what
	num_data_bytes = 0;														// Default 0 for packets with no data bytes
	uint16_t base_pkt_sz;															//size of packet not including data bytes
	
	// Handle write command from master
	if (func_code == FC_WRITE_MULT) {
		if(buffer_get_data_sz() < ABS_MIN_WRITE_PACKET_SIZE) return false;						//if the data size is less than this, we know the packet is incomplete
		num_data_bytes = rxBuffer.data[PKT_WRAP_ARND(rxBuffer.tail + WR_DATA_SIZE_IDX)];		//get supposed number of data bytes from the packet
		base_pkt_sz = ABS_MIN_WRITE_PACKET_SIZE - 1;											
		//need_crc = true;																		
	}
	
	// Handle write response from slave or read command from master (identical packet structure)
	else if (func_code == FC_READ_MULT) {
		if(buffer_get_data_sz() < WRITE_RES_PACKET_SIZE) return false;					//if the data size is less than this, we know the packet is incomplete
		base_pkt_sz = WRITE_RES_PACKET_SIZE;											//we know the final packet size
		// need_crc = true;																//still need crc because we need to confirm we are processing a complete packet and not some segment of another packet
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