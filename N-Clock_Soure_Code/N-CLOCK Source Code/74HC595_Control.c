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

#include "74HC595_Control.h"

#define	STCP	1,BIT4		// Latch
#define SHCP	1,BIT5		// Clock

void pulse(char port, char pin)
{
	switch (port)
	{
		case (1):
			P1OUT |= pin;
			P1OUT &= ~pin;
			break;
		case (2):
			P2OUT |= pin;
			P2OUT &= ~pin;
			break;
		case (3):
			P3OUT |= pin;
			P3OUT &= ~pin;
			break;
		default:
			break;
	}
}

void up(char port, char pin)
{
	switch (port)
	{
		case (1):
			P1OUT |= pin;
			break;
		case (2):
			P2OUT |= pin;
			break;
		case (3):
			P3OUT |= pin;
			break;
		default:
			break;
	}
}

void down(char port, char pin)
{
	switch (port)
	{
		case (1):
			P1OUT &= ~pin;
			break;
		case (2):
			P2OUT &= ~pin;
			break;
		case (3):
			P3OUT &= ~pin;
			break;
		default:
			break;
	}
}

void write(char num0,char num1,char num2,char num3,char num4,char num5, char led1, char led2)
{
	char i,j,data[8];
	data[0] = num0;
	data[1] = num1;
	data[2] = num2;
	data[3] = num3;
	data[4] = num4;
	data[5] = num5;
	data[6] = led1;
	data[7] = led2;
	for (i=1;i<=8;i++)
	{
		for (j=0;j<8;j++)
		{
			if ((data[j] & 0x01) == 0x00)
			{
				down(3,BIT0<<j);		// 0
			}
			if ((data[j] & 0x01) == 0x01)
			{
				up(3,BIT0<<j);			// 1
			}
			data[j] = (data[j] >> 1) & 0x7F;
		}
		pulse(SHCP);	// Get data to IC
	}
}

/******************************************************************************
 * END OF 74HC595_Control.c
*******************************************************************************/
