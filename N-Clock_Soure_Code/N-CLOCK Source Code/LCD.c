/******************************************************************************
 *
 * www.payitforward.edu.vn
 *
 ******************************************************************************/

/******************************************************************************
 *
 * TI LAUNCH PAD CODING
 *
 ******************************************************************************/

/******************************************************************************
 *
 *    Module       : LCD.C
 *    Description  : This file describes API functions that support you to
 *                   display text on YM1602C 16x2 LCD.
 *
 *  Tool           : CCS 5.1
 *  Chip           : MSP430G2xxx
 *  History        : 21-7-2012
 *  Version        : 2.1
 *
 *  Author         : Nguyen Tien Manh, CLB NCKH DDT (manhcly@ymail.com)
 *  Notes          :
 *      To apply these functions, you must include the header file lcd.h and
 *      add this code file to your project.
 *
 *      For hardware connecting, see "PINS MAPPING" below.
 *
 *      These functions just help you to "write", not "read" LCD status.
 *
******************************************************************************/

 /****************************************************************************
 * IMPORT
******************************************************************************/

#include <msp430.h>
#include "LCD.h"

 /****************************************************************************
 * DECLARATIONS
******************************************************************************/

//*****************************************************************************
// REGISTER DEFINITION
//*****************************************************************************
union reg
{    unsigned char _byte;            // byte declaration
     struct bit
       {
       unsigned char b0:1; //bit 0
       unsigned char b1:1; //bit 1
       unsigned char b2:1; //bit 2
       unsigned char b3:1; //bit 3
       unsigned char b4:1; //bit 4
       unsigned char b5:1; //bit 5
       unsigned char b6:1; //bit 6
       unsigned char b7:1; //bit 7
       } _bit;
};
//*****************************************************************************
// DECLARE MEMORY 'S ADDRESS OF GPIO REGISTERS
//*****************************************************************************\

union reg* P2_dir = (union reg*)0x2a;
union reg* P2_out = (union reg*)0x29;

union reg* P1_dir = (union reg*)0x22;
union reg* P1_out = (union reg*)0x21;

/****************************************************************************
* PINS MAPPING (!! PAY ATTENTION !!)
******************************************************************************/

// Your YM1602C 16x2 must be supplied with a voltage between 4.5V-5.5V
// However, signal pins can understand 3.3V logic level from MSP430 as '1'
// Check your hardware connecting and map it to these definitions below
// Map: LCD PIN <-> Output_Port out/dir  ->  Output_Pin

// LCD 16x2 pins map:
//		3		5		10		11		12		13
// 	   P2.0	  P2.1	   P2.2	   P2.3	   P2.4	   P2.5

#define LCD_RS         P2_out -> _bit.b0
#define LCD_RS_DIR     P2_dir -> _bit.b0
#define LCD_EN         P2_out -> _bit.b1
#define LCD_EN_DIR     P2_dir -> _bit.b1
//  This driver just help you to "write" LCD, not to read LCD "status".
// So that, you must pull your pin LCD_RW to GND (0V).
#define LCD_DATA_4     P2_out -> _bit.b2
#define LCD_DATA_4_DIR P2_dir -> _bit.b2
#define LCD_DATA_5     P2_out -> _bit.b3
#define LCD_DATA_5_DIR P2_dir -> _bit.b3
#define LCD_DATA_6     P2_out -> _bit.b4
#define LCD_DATA_6_DIR P2_dir -> _bit.b4
#define LCD_DATA_7     P2_out -> _bit.b5
#define LCD_DATA_7_DIR P2_dir -> _bit.b5

 /****************************************************************************
* MODULE PRIVATE FUNCTIONS:
* void lcd_delay_us (unsigned long t);
* void lcd_delay_ms (unsigned long t);
* void lcd_put_byte(unsigned char rs, unsigned char data);
*
******************************************************************************/

//*****************************************************************************
// Delay functions
//*****************************************************************************
void lcd_delay_us (unsigned long t)
{
	int i;
	for (i = 0; i<t; i++ )
		_delay_cycles(MCLK_F);
}
void lcd_delay_ms (unsigned long t)
{
	int i;
	for (i = 0; i<t; i++ )
		_delay_cycles(MCLK_F*1000);
}
//*****************************************************************************
// Send a byte of data (rs == 1) or command (rs == 0) to LCD
//*****************************************************************************
void lcd_put_byte(unsigned char rs, unsigned char data)
{
	LCD_RS = 0;
    if(rs) LCD_RS = 1;
    lcd_delay_us(20);
    LCD_EN = 0;

    // send the high nibble
	if (data&BIT4) LCD_DATA_4 = 1;
		else LCD_DATA_4 = 0;
	if (data&BIT5) LCD_DATA_5 = 1;
		else LCD_DATA_5 = 0;
	if (data&BIT6) LCD_DATA_6 = 1;
		else LCD_DATA_6 = 0;
	if (data&BIT7) LCD_DATA_7 = 1;
		else LCD_DATA_7 = 0;
    
    lcd_delay_us(20);
    LCD_EN = 1;
    lcd_delay_us(20);
    LCD_EN = 0;

    // send the low nibble
    if (data&BIT0) LCD_DATA_4 = 1;
    	else LCD_DATA_4 = 0;
    if (data&BIT1) LCD_DATA_5 = 1;
    	else LCD_DATA_5 = 0;
    if (data&BIT2) LCD_DATA_6 = 1;
    	else LCD_DATA_6 = 0;
    if (data&BIT3) LCD_DATA_7 = 1;
    	else LCD_DATA_7 = 0;

    lcd_delay_us(20);
    LCD_EN = 1;
    lcd_delay_us(20);
    LCD_EN = 0;
}

 /****************************************************************************
* MODULE PUPLIC FUNCTIONS:
* void lcd_init(void)
* void lcd_clear(void)
* void lcd_gotoxy(unsigned char col, unsigned char row)
* void lcd_putc(char c)
* void lcd_puts(const char* s)
* void lcd_clr_puts (const char* s1 , const char* s2)
* void lcd_put_num (unsigned long val, char dec, unsigned char neg)
******************************************************************************/

//*****************************************************************************
// Initialization for LCD module
//*****************************************************************************
void lcd_init(void)
{
	// Set all signal pins as output
	LCD_RS_DIR = 1;
	LCD_EN_DIR = 1;
	LCD_DATA_4_DIR = 1;
	LCD_DATA_5_DIR = 1;
	LCD_DATA_6_DIR = 1;
	LCD_DATA_7_DIR = 1;

    LCD_RS = 0;
    LCD_EN = 0;
    lcd_delay_ms(200);                // delay for power on

    // reset LCD
    lcd_put_byte(0,0x30);
    lcd_delay_ms(50);
    lcd_put_byte(0,0x30);
    lcd_delay_ms(50);
    lcd_put_byte(0,0x32);
    lcd_delay_ms(200);                // delay for LCD reset

    lcd_delay_ms(2);     // wait for LCD
    lcd_put_byte(0,FOUR_BIT & LINES_5X7);            // Set LCD type
    lcd_delay_ms(2);     // wait for LCD

    lcd_put_byte(0,DOFF&CURSOR_OFF&BLINK_OFF);        // display off
    lcd_delay_ms(2);     // wait for LCD
    lcd_put_byte(0,DON&CURSOR_OFF&BLINK_OFF);        // display on
    lcd_delay_ms(2);     // wait for LCD

    lcd_put_byte(0,0x01);              // clear display and move cursor to home
    lcd_delay_ms(2);     // wait for LCD
    lcd_put_byte(0,SHIFT_CUR_LEFT);                // cursor shift mode
    lcd_delay_ms(2);     // wait for LCD
    lcd_put_byte(0,0x01);              // clear display and move cursor to home
    lcd_delay_ms(2);     // wait for LCD
}

//*****************************************************************************
// Clear the LCD
//*****************************************************************************
void lcd_clear(void)
{
    lcd_put_byte(0,0x01);        // display off
    lcd_delay_ms(2);     // wait for LCD
}

//*****************************************************************************
// Move the pointer of LCD to coordinate (column, row)
// Note that cursor is at (0,0) after reset of clear LCD
//*****************************************************************************
void lcd_gotoxy(unsigned char col, unsigned char row)
{
    unsigned char address;

    if(row!=0)
        address=0x40;
    else
        address=0;

    address += col;
    lcd_put_byte(0,0x80|address);
    lcd_delay_ms(2);     // wait for LCD
}

//*****************************************************************************
// Put a writable character on LCD
// If the character is '\f', clear the LCD
// If the character is '\n', move cursor to the second line
//*****************************************************************************
void lcd_putc(char c)
{
    switch(c){
        case '\f':
            lcd_put_byte(0, 0x01);
            lcd_delay_ms(2);     // wait for LCD
            break;
        case '\n':
            lcd_gotoxy(0, 0x01);
            break;
        default:
            lcd_put_byte(1, c);
            lcd_delay_ms(2);     // wait for LCD
            break;
    }
}

//*****************************************************************************
// Put a string s[] on LCD.
// Please note that the string is ended with '/0' (or 0x00)
//*****************************************************************************
void lcd_puts(const char* s)
{
    while(*s){
        lcd_putc(*s++);
    }
}

//*****************************************************************************
// Clear LCD. Put string s1[] on line 1 and string s2[] on line 2
//*****************************************************************************
void lcd_clr_puts (const char* s1 , const char* s2)
{
    lcd_clear ();
    lcd_puts (s1);
    lcd_gotoxy (0,1);
    lcd_puts (s2);
}

//*****************************************************************************
// Put a constant or a number stored in 'val' on LCD
// The number must be multiplied and convert in type "unsigned long"
//                                                or "unsigned int"
//   dec: amount of digits after '.'
//   neg: sign of the number, neg = 0 if the number is positive
//
// Example 1: lcd_put_num(val, 3, 0); // val = 12345
//        will put "12.345" on the LCD
// Example 2: lcd_put_num(val, 6, 1); // val = 12345
//        will put "-0.012345" on the LCD
//
// Note: This function may contain some bugs. If you can detect and fix them
//       , please contact me :d
//*****************************************************************************
void lcd_put_num (unsigned long val, char dec, unsigned char neg)
{
    char i, j, digit,k;
    long total;
    long temp;
    for (i = 0, total = val; total > 0;i++) // count number
        total /= 10;
        total = i;
    
    if (neg !=0 )
      lcd_putc ('-');
    
    if ((total - dec) <=0)	// if total < dec put 0.xxx
        lcd_putc('0');
    else
    {      
    
      for (i=0; i< (total-dec); i++)
      {
        temp = 1;
        for (j=1;j<(total-i);j++)
            temp *=10;
        digit = (val/temp)%10;
        lcd_putc(digit + 0x30);
      }
    }
    if (dec > 0)
    {
      lcd_putc('.');
      if ( (dec - total)> 0)	// ex: val = 55; dec = 3 put 0.055
      {
        for ( i = 0; i < (dec-total);i++)
          lcd_putc('0');
        k = 0;
      }
      else
        k = total - dec;
      for (i=k; i< total; i++)
      {
       temp = 1;
       for (j=1;j<(total-i);j++)
           temp *=10;
       digit = (val/temp)%10;
       lcd_putc(digit + 0x30);
      }
    }    
}

/******************************************************************************
 * END OF LCD.C
*******************************************************************************/
