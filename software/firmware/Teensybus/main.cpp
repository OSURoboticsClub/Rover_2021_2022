#include <Arduino.h>
#include <modbus.h>

void setup()
{
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  modbus_init(9600, 1);
}

void loop()
{
  // put your main code here, to run repeatedly:
  modbus_update();
}