/*
 * IncFile1.h
 *
 * Created: 2/3/2022 3:37:46 PM
 *  Author: Anthony
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


#ifndef MODBUS_H_
#define MODBUS_H_

void modbus_init(Uart*, const uint32_t, Pio*, const uint32_t, const uint8_t);    // Initialize modbus uart port, clock, memory, transmit enable, and ...

void modbus_update(void);   //This function does all of the heavy lifting for modbus

#define FC_WRITE_MULTIPLE	0x10	//write multiple function code
#define FC_READ_MULTIPLE	0x03	//read multiple function code

#define RX_BUFFER_SIZE		1024	//size of RX buffer, this determines max incoming packet size
#define TX_BUFFER_SIZE		1024	//size of TX buffer, this determines max outgoing packet size

#define REGISTER_AR_SIZE	256		//Size of the register array for a given data type
#define INT_REG_OFFSET		0		//Offset for translating uint16_t array index to register index
#define FLOAT_REG_OFFSET	256		//Offset for translating float array index to register index
#define CHAR_REG_OFFSET		512		//Offset for translating char array index to register index
#define BOOL_REG_OFFSET		768		//Offset for translating bool array index to register index

#define SLAVE_ID_IDX		0		//packet byte index for slave ID byte
#define FC_IDX				1		//packet byte index for Function Code byte
#define START_REG_H_IDX		2		//packet byte index for the high side of the start register number
#define START_REG_L_IDX		3		//packet byte index for the low side of the start register number
#define END_REG_H_IDX		4		//packet byte index for the high side of the end register number
#define END_REG_L_IDX		5		//packet byte index for the low side of the end register number
#define WR_DATA_SIZE_IDX	6		//packet byte index for the size of the data to follow in bytes (write multiple only)
#define RD_DATA_SIZE_IDX	2		//packet byte index for the size of the data to follow in bytes (read multiple only)

//#define MODBUS_DEBUG				//uncomment this to enable debugging over USB_CDC this depends on USB_CDC being initialized elsewhere

//internal functions:
void UART_Handler(void);

#endif /* MODBUS_H_ */