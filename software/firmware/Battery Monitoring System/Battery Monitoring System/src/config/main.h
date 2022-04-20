/*
 * main.h
 *
 * Created: 4/19/2022 3:55:36 PM
 *  Author: Anthony
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define ADC_CLK_FREQ			500000
#define QTY_TRACKING_PERIODS	1
#define TRANSFER_PERIOD			2

#define QTY_ANALOG_SOURCES	16
#define CELLVIDXS			0
#define CELL1IDX			0
#define CELL2IDX			1
#define CELL3IDX			2
#define CELL4IDX			3
#define CELL5IDX			4
#define CELL6IDX			5
#define TEMPIDXS			6
#define TEMP1IDX			6
#define TEMP2IDX			7
#define TEMP3IDX			8
#define TEMP4IDX			9
#define TEMP5IDX			10
#define TEMP6IDX			11
#define CURRENTIDX			12
#define SHUNTTEMPIDX		13
#define FETTEMPIDX			14
#define STACKVOLTIDX		15

#define CELLV_CHANNEL		ADC_CHANNEL_0
#define CURRENT_CHANNEL		ADC_CHANNEL_1
#define STACK_CHANNEL		ADC_CHANNEL_2
#define EXTERN_TEMP_CHANNEL	ADC_CHANNEL_3
#define FET_TEMP_CHANNEL	ADC_CHANNEL_4
#define SHUNT_TEMP_CHANNEL	ADC_CHANNEL_5

#define QTY_PROTECTIONS		6
#define OVERTEMPIDX			0
#define UNDERTEMPIDX		1
#define OVERVOLTIDX			2
#define UNDERVOLTIDX		3
#define OVERCURRENTIDX		4
#define CELLIMBALANCEIDX	5

#define TC_CH  1

#define OVER_VOLTAGE_THRESHOLD		4.25;

void analogCalculate();
void adcSetup();
void pinWrite(Pio*, const uint32_t, bool);
void triggerProtections();
bool overTemp();
bool underTemp();
bool overVolt();
bool underVolt();
bool overCurrent();
bool cellImbalance();
void protec();

#endif /* MAIN_H_ */