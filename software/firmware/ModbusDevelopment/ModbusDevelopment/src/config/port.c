#include <port.h>

#ifdef TEENSYBUS

HardwareSerial *port = NULL;
usb_serial_class *sw_port = NULL;
uint16_t timeout;

static void _begin(uint32_t baud)
{
    if (sw_port)
        sw_port->begin(baud);
    else
        port->begin(baud);
}

static int _read()
{
    return sw_port ? sw_port->read() : port->read();
}

static void _write(uint8_t *packet, uint16_t packetSize)
{
    if (sw_port)
        sw_port->write(packet, packetSize);
    else    
        port->write(packet, packetSize);
}

static int _available()
{
    return sw_port ? sw_port->available() : port->available();
}

void portSetup(uint8_t serialNumber, uint8_t TXEnablePin, const uint32_t baud, const uint16_t serialTimeout)
{

	timeout = serialTimeout;

    switch (serialNumber)
    {
    case 1:
        port = &Serial1;
        break;
    case 2:
        port = &Serial2;
        break;
    case 3:
        port = &Serial3;
        break;
    case 0:
	sw_port = &Serial;
	break;
    default:
        break;
    }

	_begin(baud);
	if (port && TXEnablePin > 1)
	{
		pinMode(TXEnablePin, OUTPUT);
		digitalWrite(TXEnablePin, LOW);
		port->transmitterEnable(TXEnablePin);
	}
}

void portWrite(uint8_t *packet, uint16_t packetSize)
{
	_write(packet, packetSize);
}

// interrupt handler for incoming data
void serialEventHandler()
{
    while (_available())
    { // confirm there is data ready to be read
        rxBuffer.data[rxBuffer.head] = _read();
        rxBuffer.head = PKT_WRAP_ARND(rxBuffer.head + 1); // iterate the head through the ring buffer
    }
}

void serialEvent()
{
	serialEventHandler();
}

void serialEvent1()
{
	serialEventHandler();
}

void serialEvent2()
{
	serialEventHandler();
}

void serialEvent3()
{
	serialEventHandler();
}

#endif

#ifdef ATMELBUS

Uart *RS485Port;
Pio *globalEnPinPort;
uint32_t globalEnPin;

uint16_t transmitIndex; // helper variables for transmitting

void portSetup(Uart *port485, const uint32_t baud, Pio *enPinPort, const uint32_t enPin)
{
	RS485Port = port485;

	if (RS485Port == UART0)
	{
		pmc_enable_periph_clk(ID_UART0);                  // Enable the clocks to the UART modules
		pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA9);  // Sets PA9 to RX
		pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA10); // Sets PA10 to TX
		NVIC_EnableIRQ(UART0_IRQn);                       // enables interrupts related to this port
	}

	if (RS485Port == UART1)
	{
		pmc_enable_periph_clk(ID_UART1);                 // Enable the clocks to the UART modules
		pio_set_peripheral(PIOB, PIO_PERIPH_A, PIO_PB2); // Sets PB2 to RX
		pio_set_peripheral(PIOB, PIO_PERIPH_A, PIO_PB3); // Sets PB3 to TX
		NVIC_EnableIRQ(UART1_IRQn);                      // enables interrupts related to this port
	}

	uint32_t clockSpeed = sysclk_get_peripheral_bus_hz(RS485Port); // gets CPU speed to for baud counter

	sam_uart_opt_t UARTSettings = {
		.ul_baudrate = baud,                               // sets baudrate
		.ul_mode = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO, // sets to normal mode
		.ul_mck = clockSpeed                               // sets baud counter clock
	};

	uart_init(RS485Port, &UARTSettings); // init the UART port
	uart_enable_rx(RS485Port);
	uart_enable_tx(RS485Port);
	uart_enable_interrupt(RS485Port, UART_IER_RXRDY); // Enable interrupt for incoming data

	pio_set_output(enPinPort, enPin, LOW, DISABLE, DISABLE); // init the enable pin
	globalEnPinPort = enPinPort;
	globalEnPin = enPin;
}

void portWrite(uint8_t *packet, uint16_t packetSize)
{
	// write out response packet
	pio_set(globalEnPinPort, globalEnPin); // transceiver transmit enable
	transmitIndex = 0;
	uart_enable_interrupt(RS485Port, UART_IMR_TXRDY);
}

// interrupt handler for incoming data
void UART_Handler(void)
{
	if (uart_is_rx_ready(RS485Port))
	{                                                          // confirm there is data ready to be read
		uart_read(RS485Port, &(rxBuffer.data[rxBuffer.head])); // move the data into the next index of the rx buffer
		rxBuffer.head = PKT_WRAP_ARND(rxBuffer.head + 1);      // iterate the head through the ring buffer
	}
	else if (uart_is_tx_ready(RS485Port))
	{
		if (transmitIndex < responsePacketSize)
		{
			uart_write(RS485Port, responsePacket[transmitIndex]);
			transmitIndex++;
		}
		else if (uart_is_tx_empty(RS485Port))
		{
			pio_clear(globalEnPinPort, globalEnPin);
			uart_disable_interrupt(RS485Port, UART_IMR_TXRDY);
		}
	}
}

// Regardless of what UART port triggers the interrupt, the behavior is the same
void UART0_Handler()
{
	UART_Handler();
}

void UART1_Handler()
{
	UART_Handler();
}

#endif