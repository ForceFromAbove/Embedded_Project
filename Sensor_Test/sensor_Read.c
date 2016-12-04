/*
 * sensor_Read.c
 *
 *  Created on: Dec 3, 2016
 *      Author: AaronEwing
 */



#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

bool OMGsomethingworksflag = 0;


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
	uint16_t i = 0x00;		// funky things at 0xC4=i / 0x9A=j
	uint16_t j = 0x00;


for (i = 0x00; i <= 0xFF; i++ ) {
	for (j = 0x00; j <= 0xFF; j++ ) {
	while (UCB1CTL1 & UCTXSTP);               				// Ensure stop condition got sent

	UCB1CTL1 |= UCTR + UCTXSTT;								// START
	while ((UCB1CTL1 & UCTXSTT) || !(UCB1IFG & UCTXIFG0));
	//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS

	UCB1TXBUF = i;
	while (!(UCB1IFG & UCTXIFG0));
	//UCB1TXBUF = registerAddress;							// MEASURE COMMAND
	UCB1TXBUF = j;

	if (i > 0xC0) {
			__delay_cycles(100000);
		}
	__delay_cycles(500);

	while (!(UCB1IFG & UCTXIFG0));
	//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS
	UCB1TXBUF = i;


	if (i > 0xC0) {
		__delay_cycles(100000);
	}
	__delay_cycles(10000);
	}
}

/*
	for (i = 0x00; i <= 0xFF; i++ ) {
		for (j = 0x00; j <= 0xFF; j++ ) {
		while (UCB1CTL1 & UCTXSTP);               				// Ensure stop condition got sent

		UCB1CTL1 |= UCTR + UCTXSTT;								// START
		while ((UCB1CTL1 & UCTXSTT) || !(UCB1IFG & UCTXIFG0));
		//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS

		UCB1TXBUF = i;
		while (!(UCB1IFG & UCTXIFG0));
		//UCB1TXBUF = registerAddress;							// MEASURE COMMAND
		UCB1TXBUF = j;

		if (i > 0xC0) {
				__delay_cycles(100000);
			}
		__delay_cycles(500);

		while (!(UCB1IFG & UCTXIFG0));
		//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS
		UCB1TXBUF = i;


		if (i > 0xC0) {
			__delay_cycles(100000);
		}
		__delay_cycles(10000);

		while (!(UCB1IFG & UCTXIFG0));
		//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS
		UCB1TXBUF = i;

		if (i > 0xC0) {
			__delay_cycles(100000);
		}
		__delay_cycles(1000000);

		while (!(UCB1IFG & UCTXIFG0));
		//UCB1TXBUF = slaveAddress;								// SLAVE ADDRESS
		UCB1TXBUF = i;
		}
	}
	*/

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

#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void) {

	OMGsomethingworksflag = 1;

  switch(__even_in_range(UCB1IV, USCI_I2C_UCBIT9IFG)) {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:                 // Vector 8: STPIFG
    //  TXData = 0;
      UCB0IFG &= ~UCSTPIFG;                 // Clear stop condition int flag
      break;
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:  break;         // Vector 22: RXIFG0
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
 //     UCB0TXBUF = TXData++;
      break;
    case USCI_I2C_UCBCNTIFG: break;         // Vector 26: BCNTIFG
    case USCI_I2C_UCCLTOIFG: break;         // Vector 28: clock low timeout
    case USCI_I2C_UCBIT9IFG: break;         // Vector 30: 9th bit
    default: break;
  }
}
