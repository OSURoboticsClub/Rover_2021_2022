/*
 * registers.h
 *
 * Created: 8/7/2022 7:41:52 PM
 *  Author: Anthony Grana
 */ 


#ifndef REGISTERS_H_
#define REGISTERS_H_


// modbus settings
#define SLAVEID 15

#define MODBUS_SLAVE_ID 15
#define MODBUS_BPS 115200
#define MODBUS_TIMEOUT 2000
#define MODBUS_SER_PORT UART0
#define MODBUS_EN_PORT PIOA
#define MODBUS_EN_PIN PIO_PA11

//int Registers
#define stepperPosition 1
#define cameraSelect 2
#define lazerEnable 3
#define limSw1	4
#define limSw2	5
#define limSw3	6
#define limSw4	7

//float Registers


//char Registers


//Bool Registers



#endif /* REGISTERS_H_ */