#include <stdint.h>

#define TEENSYBUS
//#define ATMELBUS

#define RX_BUFFER_SIZE 8192 // size of RX buffer, this determines max incoming packet size
#define TX_BUFFER_SIZE 8192 //							!!!!!! Important change this value to the size of the tx buffer in the UART module

// To handle edge case if a packet starts toward end of buffer but wraps around to beginning
#define PKT_WRAP_ARND(idx) \
    ((idx) & (RX_BUFFER_SIZE - 1))

struct ringBuffer
{                  // ring buffer to serve as rx buffer. Helps solve lots of data shifting problems
    uint16_t head; // Next free space in the buffer
    uint16_t tail; // Start of unprocessed data and beginning of packet
    uint8_t data[RX_BUFFER_SIZE];
};

extern struct ringBuffer rxBuffer;

extern uint8_t responsePacket[TX_BUFFER_SIZE];
extern uint16_t responsePacketSize;

extern uint16_t timeout;

#ifdef TEENSYBUS

#include <Arduino.h>

extern HardwareSerial *port;

void portSetup(uint8_t, uint8_t, const uint32_t, const uint16_t);

void portWrite(uint8_t *, uint16_t);

// interrupt handler for incoming data
void serialEvent();

#endif

#ifdef ATMELBUS

#include <asf.h>

void portSetup(Uart *, const uint32_t, Pio *, const uint32_t);

void portWrite(uint8_t *, uint16_t);

void UART_Handler(void);

void UART0_Handler();

void UART1_Handler();

uint32_t millis(void);

#endif