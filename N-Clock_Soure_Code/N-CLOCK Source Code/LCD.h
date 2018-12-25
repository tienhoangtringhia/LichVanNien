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
 *    Module       : LCD.H
 *    Description  : This header file includes definitions and prototypes of
 *          API functions that support you to display text on YM1602C 16x2 LCD.
 *
 *  Tool           : CCS 5.1
 *  Chip           : MSP430G2xxx
 *  History        : 21-7-2012
 *  Version        : 2.1
 *
 *  Author         : Nguyen Tien Manh, CLB NCKH DDT (manhcly@ymail.com)
 *  Notes          :
 *      To apply these functions, you must include this header file and add
 *      the lcd.c file to your project.
 *      For hardware connecting, see "PINS MAPPING" in LCD.C
 *
******************************************************************************/

#ifndef LCD_H_
#define LCD_H_

 /****************************************************************************
 * USER DEFINITIONS
******************************************************************************/

#ifndef MCLK_F
#define MCLK_F 1 // frequency of Master Clock in MHz
#endif
// This definition supports to lcd delay functions. You should change it to the
// right MCLK frequency that you configure through "Config_Clocks" function.

 /****************************************************************************
 * HARD DEFINITIONS
******************************************************************************/

/* Display ON/OFF Control definitions */
#define DON          0x0F  /* Display on      */
#define DOFF         0x0B  /* Display off     */
#define CURSOR_ON    0x0F  /* Cursor on       */
#define CURSOR_OFF   0x0D  /* Cursor off      */
#define BLINK_ON     0x0F  /* Cursor Blink    */
#define BLINK_OFF    0x0E  /* Cursor No Blink */

/* Cursor or Display Shift definitions */
#define SHIFT_CUR_LEFT     0x04  /* Cursor shifts to the left   */
#define SHIFT_CUR_RIGHT    0x05  /* Cursor shifts to the right  */
#define SHIFT_DISP_LEFT    0x06  /* Display shifts to the left  */
#define SHIFT_DISP_RIGHT   0x07  /* Display shifts to the right */

/* Function Set definitions */
#define FOUR_BIT    0x2C  /* 4-bit Interface               */
#define EIGHT_BIT   0x3C  /* 8-bit Interface               */
#define LINE_5X7    0x30  /* 5x7 characters, single line   */
#define LINE_5X10   0x34  /* 5x10 characters               */
#define LINES_5X7   0x38  /* 5x7 characters, multiple line */

 /****************************************************************************
* FUNCTIONS 'S PROTOTYPES
******************************************************************************/
// For further description, see LCD.c
void lcd_delay_us (unsigned long t);
void lcd_delay_ms (unsigned long t);
void lcd_put_byte(unsigned char rs, unsigned char data);

void lcd_init(void);
void lcd_clear(void);
void lcd_gotoxy(unsigned char col, unsigned char row);
void lcd_putc(char c);
void lcd_puts(const char* s);
void lcd_clr_puts (const char* s1 , const char* s2);
void lcd_put_num (unsigned long val, char dec, unsigned char neg);


#endif /* LCD_H_ */
/******************************************************************************
 * END OF standard LCD.H
*******************************************************************************/
