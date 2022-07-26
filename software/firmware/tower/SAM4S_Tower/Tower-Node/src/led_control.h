#ifndef LED_CONTROL_H
#define LED_CONTROL_H

// The LEDs are managed with a single LED control register
// A later FastLED port will then use this register to create a bunch of cool LED effects on the Neopixel
#define LED_CONTROL_REG 0


// Reads the appropriate modbus register for LED control and uses certain bits (starting at 0) to configure the LEDs
void set_LEDs(void);


#endif