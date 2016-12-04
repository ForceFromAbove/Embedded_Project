/*
 * sensor_Read.c
 *
 *  Created on: Dec 3, 2016
 *      Author: AaronEwing
 */

#include <msp430.h>
#include <stdint.h>

	uint16_t temp_Conversion (raw_Temp) {
	uint32_t hold_Value = 0;
	uint16_t temperature = 0;

	hold_Value = 175.72*raw_Temp;
	hold_Value = hold_Value/65536;
	hold_Value = hold_Value - 46.85;
	temperature = hold_Value;// << 16;

	return temperature;
}

void init_i2c (void) {

	P3SEL0 |= BIT1 + BIT2;     					// Assign I2C pins to USCI_B1
	P3SEL0 &= ~(BIT0 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
	P3SEL1 |= BIT0 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
	P3SEL1 &= ~(BIT1 + BIT2);     				// Assign I2C pins to USCI_B1

	UCB1CTLW0 |= UCSWRST;                      				// Enable SW reset
	UCB1CTLW0 |= UCMST + UCMODE_3 + UCSYNC + UCSSEL_2; // I2C Master, synchronous mode
	UCB0BRW = 0x0016;                             	// fSCL = 1Mhz/100 = ~100kHz
	UCB1BRW = 80;
	UCB1CTLW0 &= ~UCSWRST;                 	// **Initialize USCI state machine**
}


uint16_t read_Sensor (uint8_t slaveAddress, uint8_t registerAddress) {

	UCB1I2CSA = slaveAddress;
	uint8_t dataMSB = 0x00;
	uint8_t dataLSB = 0x00;
	uint16_t raw_Temp;
	uint16_t temperature = 0;


	while (UCB1CTL1 & UCTXSTP);               				// Ensure stop condition got sent

	UCB1CTL1 |= UCTR + UCTXSTT;
	// START
	while ((UCB1CTL1 & UCTXSTT) || !(UCB1IFG & UCTXIFG0));
	UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS
	while (!(UCB1IFG & UCTXIFG0));
	UCB1TXBUF = registerAddress;							// MEASURE COMMAND
	while (!(UCB1IFG & UCTXIFG0));
	UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS

	while (!(UCB1IFG & UCRXIFG0));
	dataMSB = UCB1RXBUF;
	while (!(UCB1IFG & UCRXIFG0));
	dataLSB = UCB1RXBUF;

	raw_Temp = dataMSB;
	raw_Temp = raw_Temp << 8;
	raw_Temp = raw_Temp + dataLSB;

	temperature = temp_Conversion (raw_Temp);

	return temperature;
}

