/*
 * IncFile1.h
 *
 * Created: 2/3/2022 3:37:46 PM
 *  Author: Anthony Grana,
			Blake Hakkila,
			Kurtis Dinelle
 */

/**********************************************************************
 *REQUIREMENTS
 ***********************************************************************
 *In ASF Wizard include UART (not USART)
 *(Debuging only) in ASF Wizard include USB Device in CDC mode and run udc_start(); in init.c
 */

/**********************************************************************
 *
 *	HOW THIS MODBUS LIBRARY WORKS
 *
 *this is a SLAVE ONLY interrupt based modbus library that supports the following function codes:
 *0x03 Read multiple holding registers
 *0x10 Write multiple holding registers
 *
 *This library supports a non-standard extended register range up 0-1023 with different data types
 *0-255		Int Registers
 *256-511	Float Registers
 *512-767	Char Registers
 *768-1023	Bool Registers
 *
 *This structure was specifically chosen to maintain backwards compatibility with the rover's systems from 2017-2022
 *
 *	INSTRUCTIONS
 *inside of init.c run the modbus_init(); function
 *Inside of a fast opperating main loop run the function modbus_update();
 ***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <port.h>

#ifndef MODBUS_H_
#define MODBUS_H_

#define REGISTER_AR_SIZE 256 // Size of the register array for a given data type

extern uint16_t intRegisters[REGISTER_AR_SIZE];
extern float floatRegisters[REGISTER_AR_SIZE];
extern char charRegisters[REGISTER_AR_SIZE];
extern bool boolRegisters[REGISTER_AR_SIZE];

bool communicationGood(void);

void modbus_init(const uint8_t); // Initialize modbus uart port, clock, memory, transmit enable, and ...

void modbus_update(void); // This function does all of the heavy lifting for modbus

#define FC_WRITE_MULT 0x10 // write multiple registers function code
#define FC_READ_MULT 0x03  // read multiple registers function code

#define INT_REG_OFFSET 0	 // Offset for translating uint16_t array index to register index
#define FLOAT_REG_OFFSET 256 // Offset for translating float array index to register index
#define CHAR_REG_OFFSET 512	 // Offset for translating char array index to register index
#define BOOL_REG_OFFSET 768	 // Offset for translating bool array index to register index

#define INT_REG_BYTE_SZ 2	// Number of bytes for an int register
#define FLOAT_REG_BYTE_SZ 4 // Number of bytes for a float register
#define CHAR_REG_BYTE_SZ 1	// Number of bytes for a char register
#define BOOL_REG_BYTE_SZ 1	// Number of bytes for a bool register

#define SLAVE_ID_IDX 0	   // packet byte index for slave ID byte
#define FC_IDX 1		   // packet byte index for Function Code byte
#define START_REG_H_IDX 2  // packet byte index for the high side of the start register number
#define START_REG_L_IDX 3  // packet byte index for the low side of the start register number
#define NUM_REG_H_IDX 4	   // packet byte index for the high side of the end register number
#define NUM_REG_L_IDX 5	   // packet byte index for the low side of the end register number
#define WR_DATA_SIZE_IDX 6 // packet byte index for the size of the data to follow in bytes (write multiple only)
#define RD_DATA_SIZE_IDX 2 // packet byte index for the size of the data to follow in bytes (read multiple only)

#define WR_DATA_BYTE_START 7 // packet byte index for the start of the data to be written (write multiple only)
#define RD_DATA_BYTE_START 3 // packet byte index for start of data in read response (read multiple only

#define WR_RESP_PACKET_SIZE 8	  // packet size for write multiple response packet
#define RD_RESP_PACKET_MIN_SIZE 5 // packet size for read multiple response with no data bytes added yet

#define ABS_MIN_PACKET_SIZE 7		   // this is the smallest possible packet size in the protocol in bytes
#define ABS_MIN_WRITE_PACKET_SIZE 10   // this is the smallest possible packet size for a write command from the master
#define WRITE_RES_PACKET_SIZE 8		   // this is the only possible packet size for a write response from the slave This is the same as the size of read from master
#define ABS_MIN_READ_RES_PACKET_SIZE 6 // this is the smallest possible packet size for a read response from the slave

#define CRC_SIZE 2 // size of CRC in bytes

#define MASTER_ADRESS 0X00

//#define MODBUS_DEBUG				//uncomment this to enable debugging over USB_CDC this depends on USB_CDC being initialized elsewhere

// internal functions:
uint8_t *pop_packet();
bool packet_complete();
uint16_t ModRTU_CRC(uint8_t *buf, int len);
void readHandler(uint8_t *responsePacket, uint16_t start_reg, uint16_t end_reg);
void writeHandler(uint8_t *data_packet, uint16_t start_reg, uint16_t end_reg);
uint16_t getReadResponseDataSize(uint16_t start_reg, uint16_t end_reg);
uint16_t buffer_get_data_sz(void);

#endif /* MODBUS_H_ */