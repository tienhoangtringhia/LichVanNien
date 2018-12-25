/****************************************************************
 *
 * 				PAY IT FORWARD COMMUNITY
 * 				www.payitforward.edu.vn
 *
 ****************************************************************/

/****************************************************************
 *
 * 				MSP430 Training Course
 *
 ****************************************************************/

/****************************************************************
 *
 *	Module		: N_Clock_main.c
 *	Description	: Some functions use to control IC 74HC595 manually
 *  Tool		: Code Composer Studio Version: 5.2.0.00033
 *	Chip		: MSP430G2553
 *	Author		: Le Tri Thong (kisseshacker@gmail.com)
 *	Notes		: 2 GPIO pins for clock & latch
 *				  for each 74HC595, use 1 GPIO pin for data
 *
 ****************************************************************/

#include <msp430g2553.h>

#ifndef SPI_H_
#define SPI_H_

void pulse(char port, char pin);
void up(char port, char pin);
void down(char port, char pin);
void write(char num0,char num1,char num2,char num3,char num4,char num5, char led1, char led2);

#endif

/******************************************************************************
 * END OF 74HC595_Control.h
*******************************************************************************/
