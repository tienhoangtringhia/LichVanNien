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
 *	Description	: Main code for N-Clock v1.0 project
 *  Tool		: Code Composer Studio Version: 5.2.0.00033
 *	Chip		: MSP430G2553
 * 	Last code	: 8th Feb 2013
 *	Author		: Le Tri Thong (kisseshacker@gmail.com)
 *	Notes		: To be developed
 *				  First day of week: Monday
 *
 ****************************************************************/

/****************************************************************
 * IMPORT
 ****************************************************************/

#include <msp430g2553.h>
#include "LCD.h"
#include "I2C.h"
#include "74HC595_Control.h"

/****************************************************************
 * DEFINE
 ****************************************************************/

#define	STCP	1,BIT4		// Latch data
#define Vref 	3.3			// Define Vcc voltage for ADC when select Vref = VCC
			//	 123456789012345 : LCD text's length
#define menu_1	"1.Set Alarm    "
#define menu_2	"2.Set Date&Time"
#define menu_3	"3.Setting      "
#define menu_4	"4.Hardware Test"
#define menu_5	"5.About        "
#define menu_6	"6.Contact      "
#define d 		user_write[4]
#define m 		user_write[5]		// Shorten checking code
#define y		user_write[6]

/*****************************************************************
 *GLOBAL VARIABLES
 ******************************************************************/

typedef char * string;
string menu[6] = {menu_1,menu_2,menu_3,menu_4,menu_5,menu_6};
string week_days[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
unsigned long adc_result0, volt0, adc_result1, volt1, adc_result3, volt3;
unsigned char led7large[] = {160,243,100,97,51,41,40,227,32,33};		// Led 7 Large number 0 -> 9
unsigned char led7large_test[] = {239,247,251,253,254,191,127,223};
unsigned char led7small[] = {68,95,98,74,89,200,192,94,64,72};			// Led 7 Small number 0 -> 9
unsigned char led7small_test[] = {254,127,223,239,247,253,251,191};
unsigned char datawrite[] = {0x00,0x09,0x20,0x07,0x17,0x02,0x13};		// Time value to set to DS1307
						//	 Sec  Min  Hour Day  Date Mon  Year
unsigned char user_set_digits[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char digits_max[13] = 		{2,9,5,9,5,9,7,3,9,1,9,9,9};
unsigned char dataread[8],user_write[7],alarm_time[4] = {0,0,0,0x24};
unsigned char cursor_x[15] = {4,5,7,8,10,11,0,5,6,8,9,13,14};			// LCD Cursor's x position in "Set time"
unsigned char hourL,minuteL,dateL,monthL,yearL,tempL,hourH,minuteH,dateH,monthH,yearH,tempH,led1 = 0xAA,led2 = 0xFF;
unsigned char ADC_select,button = 0,menu_select = 1,sub_menu_select = 1,blink;
unsigned char i,temp,in_main_menu = 0,in_sub_menu = 0,in_set_time = 0,timetype24h = 1,alarm_set = 0,alarm_beep = 0,check_time_ok = 0;

/*****************************************************************
 * PROTOTYPES
 ******************************************************************/

void Port_Init(void);
void Timer_Init(void);
void ADC10_Init(void);
void Read_Keypad(void);
void Read_LM35(void);
void Read_LDR(void);
void LCD_Write_Date_Time(void);
void LCD_Write_Keypad(char col, char row);
void LCD_Write_LM35(char col,char row);
void LCD_Write_LDR(char col,char row);
void LCD_Standby(void);
void LCD_Menu(char pos);
void LCD_Menu_Control(void);
void LCD_Set_Alarm(void);
void LCD_Edit_Date_Time(void);
void LCD_Setting(void);
void LCD_Hardware_Test(void);
void LCD_About(void);
void LCD_Contact(void);
void Calculate_Time(void);
void Check_Alarm(void);
void Alarm_Ring();

/****************************************************************
 * MAIN
 ****************************************************************/

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer

	Port_Init();
	Timer_Init();
	lcd_init();
	i2c_init(0x68);	// DS1307
	_delay_cycles(1000);
	ADC10_Init();

	lcd_clear();
	lcd_puts("Kisses Hacker");
	_delay_cycles(1000000);
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts("   Press OK     ");
	lcd_gotoxy(0,1);
	lcd_puts(" to go to menu  ");
//	i2c_write(0x00,7,datawrite);			// Set time for DS1307
	while(1)
	{
		i2c_read(0x00,8,dataread);			// Read time from DS1307
		_delay_cycles(1000);
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		Read_Keypad();
		Read_LM35();
		Read_LDR();
		LCD_Menu_Control();
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
	}
}


/*****************************************************************
* ROUTINES
******************************************************************/

/************************* CALCULATE *****************************/

void Calculate_Time(void)	// Each 62500 cycles (x8 = 0.5s)
{
	// Led1: day (Mon, Tue,...) & Alarm
	led1 = ~(0x01 << (8 - (dataread[4] % 16)));
	if (alarm_set == 1)
		led1 &= 0xFE;
	if (alarm_set == 0)
		led1 |= 0x01;
	// Led2: AM/PM, Time dot & AL
	if (timetype24h == 0)
	{
		if (dataread[3] > 0x12)
		{
			dataread[3] -= 0x12;
			if (dataread[3] == 0x0E)	// 2 special state, 20h & 21h
				dataread[3] = 0x08;		// 	  must be defined here
			if (dataread[3] == 0x0F)
				dataread[3] = 0x09;
			led2 &= ~BIT6;			// PM
		}
		else
			led2 &= ~BIT7;			// AM
	}
	if (timetype24h == 1)
		led2 |= BIT6 + BIT7;
	led2 ^= 0x10;		// Time dots
	led2 |= 0x20;		// AL
	hourL 	= led7large[dataread[3] % 16];
	hourH 	= led7large[dataread[3] / 16];
	minuteL = led7large[dataread[2] % 16];
	minuteH = led7large[dataread[2] / 16];
	dateL 	= led7small[dataread[5] % 16];
	dateH	= led7small[dataread[5] / 16];
	monthL 	= led7small[dataread[6] % 16];
	monthH	= led7small[dataread[6] / 16];
	yearL	= led7small[dataread[7] % 16];
	yearH	= led7small[dataread[7] / 16];
	tempL 	= led7small[dataread[1] % 16];
	tempH 	= led7small[dataread[1] / 16];
}

void Check_Alarm(void)
{
	i2c_read(0x10,4,alarm_time);
	_delay_cycles(1000);
	if (alarm_time[3] != 0x24)
		alarm_set = 1;
	if (alarm_time[3] == 0x24)
		alarm_set = 0;
	if (alarm_time[3] == dataread[3])			// Hour
		if (alarm_time[2] == dataread[2])		// Minute
			if (alarm_time[1] == dataread[1])	// Second
				alarm_beep = 1;
}

/************************* LCD DISPLAY ***************************/

void LCD_Standby(void)
{
	lcd_gotoxy(0,0);
	lcd_puts("   Press OK     ");
	lcd_gotoxy(0,1);
	lcd_puts(" to go to menu  ");
	if ((alarm_beep == 1) & (button != 0))
	{
		lcd_gotoxy(0,0);
		lcd_puts("   Alarm OFF    ");
		lcd_gotoxy(0,1);
		lcd_puts("----------------");
		user_write[2] = 0x24;
		i2c_write(0x10,3,user_write);		// Set off alarm (24:xx:xx) to DS1307's RAM
		alarm_beep = 0;
		_delay_cycles(2000000);
	}
	if (button == 'o')
	{
		in_main_menu = 1;
		button = 0;
		_delay_cycles(100000);
	}
}

void LCD_Menu(char pos)
{
	lcd_gotoxy(1,0);
	lcd_puts(menu[pos-1]);
	lcd_gotoxy(1,1);
	lcd_puts(menu[pos]);
}

void LCD_Menu_Control(void)		// Must have read keypad first
{
	if (in_main_menu == 0)
		LCD_Standby();
	if ((in_sub_menu == 0) & (in_main_menu == 1))		// If being in sub-menu, stay there and wait (nothing to do here)
	{
		if ((alarm_beep == 1) & (button != 0))
		{
			lcd_gotoxy(0,0);
			lcd_puts("   Alarm OFF    ");
			lcd_gotoxy(0,1);
			lcd_puts("----------------");
			user_write[2] = 0x24;
			i2c_write(0x10,3,user_write);		// Set off time (24:xx:xx) to DS1307's RAM
			alarm_beep = 0;
			_delay_cycles(2000000);
		}
		// Main menu list show
		if (button == 'a')
			in_main_menu = 0;
		if ((button == 'd') & (menu_select < 7))
			menu_select++;
		if ((button == 'u') & (menu_select > 0))
			menu_select--;
		if ((0 < menu_select) & (menu_select <= 2))
			LCD_Menu(1);
		if ((2 < menu_select) & (menu_select <= 4))
			LCD_Menu(3);
		if (menu_select == 7)			// 7 scroll back to 1
			menu_select = 1;
		if (menu_select == 0)			// 0 scroll up to 6
			menu_select = 6;
		if (menu_select > 4)
			LCD_Menu(5);
		if ((menu_select % 2) == 1)
		{
			lcd_gotoxy(0,0);
			lcd_putc(126);
			lcd_gotoxy(0,1);
			lcd_putc(' ');
		}
		else
		{
			lcd_gotoxy(0,0);
			lcd_putc(' ');
			lcd_gotoxy(0,1);
			lcd_putc(126);
		}
		// Press OK to choose an item
		if (button == 'o')
		{
			switch (menu_select)
			{
				case 1:					// Set Alarm
					in_sub_menu = 10;
					LCD_Set_Alarm();
					break;
				case 2:					// Edit date & time
					in_sub_menu = 20;
					LCD_Edit_Date_Time();
					break;
				case 3:					// Setting
					in_sub_menu = 30;
					LCD_Setting();
					break;
				case 4:					// Hardware test
					in_sub_menu = 40;
					LCD_Hardware_Test();
					break;
				case 5:					// About
					in_sub_menu = 50;
					LCD_About();
					break;
				case 6:					// Contact
					in_sub_menu = 60;
					LCD_Contact();
					break;
			}
		}
		_delay_cycles(50000);
	}
}

void LCD_Set_Alarm(void)
{
	lcd_gotoxy(0,0);
	lcd_puts("Set alarm time: ");
	lcd_gotoxy(0,1);
	lcd_puts("    ");
	lcd_putc((dataread[3] / 16) % 4 + 0x30);	// 2 bits high of hour
	lcd_putc(dataread[3] % 16 + 0x30);			// 4 bits low of hour
	lcd_putc(':');
	lcd_putc(dataread[2] / 16 + 0x30);			// 4 bits high of minute
	lcd_putc(dataread[2] % 16 + 0x30);			// 4 bits low of minute
	lcd_putc(':');
	lcd_putc(dataread[1] / 16 + 0x30);			// 4 bits high of second
	lcd_putc(dataread[1] % 16 + 0x30);			// 4 bits low of second
	lcd_puts("    ");
	user_set_digits[0] = dataread[3] / 16;
	user_set_digits[1] = dataread[3] % 16;
	user_set_digits[2] = dataread[2] / 16;
	user_set_digits[3] = dataread[2] % 16;
	user_set_digits[4] = dataread[1] / 16;
	user_set_digits[5] = dataread[1] % 16;
	i = 0;		// Cursor's step (0 -> 5)
	while(in_sub_menu != 0)
	{
		// Read Keypad
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		TA0CCTL1 = CCIE;
		Read_Keypad();
		if (button == 'a')
		{
			TA0CCTL1 &= ~CCIE;
			in_sub_menu = 0;
		}
		//  Edit alarm time in LCD
		if (button == 'r')
		{
			if (i == 6)
				i = 0;
			lcd_gotoxy(cursor_x[i],1);
			lcd_putc(user_set_digits[i] + 0x30);
			i++;
		}
		if (button == 'l')
		{
			lcd_gotoxy(cursor_x[i],1);
			lcd_putc(user_set_digits[i] + 0x30);
			i--;
		}
		if (button == 'u')
		{
			user_set_digits[i]++;
			if (user_set_digits[i] > digits_max[i])
				user_set_digits[i] = 0;
		}
		if (button == 'd')
		{
			if (user_set_digits[i] > 0)
				user_set_digits[i]--;
			if (user_set_digits[i] == 0)
				user_set_digits[i] = digits_max[i];
		}
		lcd_gotoxy(cursor_x[i],1);
		if (i == 6)
			i = 0;
		lcd_putc(user_set_digits[i] + 0x30);
		if (button == 'o')
		{
			TA0CCTL1 &= ~CCIE;
			// Re-calculate time
			user_write[0] = (user_set_digits[4] << 4) + user_set_digits[5];		// Second
			user_write[1] = (user_set_digits[2] << 4) + user_set_digits[3];		// Minute
			user_write[2] = (user_set_digits[0] << 4) + user_set_digits[1];		// Hour
			// Check alarm time
			check_time_ok = 1;
			if (user_write[2] > 0x24)	// Hour <=23, set 24 for turn off alarm
			{
				lcd_gotoxy(0,0);
				lcd_puts("Hour is 0h->23h ");
				lcd_gotoxy(0,1);
				lcd_puts("If 24: alarm OFF");
				check_time_ok = 0;
				_delay_cycles(2000000);
			}
			if (user_write[2] == 0x24)	// Hour <=23, set 24 for turn off alarm
			{
				lcd_gotoxy(0,0);
				lcd_puts("   Alarm OFF    ");
				lcd_gotoxy(0,1);
				lcd_puts("----------------");
				check_time_ok = 0;
				i2c_write(0x10,3,user_write);		// Set off time (24:xx:xx) to DS1307's RAM
				_delay_cycles(1000000);
			}
			if (check_time_ok == 0)
				in_sub_menu = 0;
			// If check is ok, set alarm time
			if (check_time_ok == 1)
			{
				alarm_set = 1;
				lcd_gotoxy(0,0);
				lcd_puts("   Alarm Set!   ");
				lcd_gotoxy(0,1);
				lcd_puts(" Time: ");
				lcd_putc(user_set_digits[0] + 0x30);
				lcd_putc(user_set_digits[1] + 0x30);
				lcd_putc(':');
				lcd_putc(user_set_digits[2] + 0x30);
				lcd_putc(user_set_digits[3] + 0x30);
				lcd_putc(':');
				lcd_putc(user_set_digits[4] + 0x30);
				lcd_putc(user_set_digits[5] + 0x30);
				lcd_putc(' ');
				in_sub_menu = 0;
				i2c_write(0x10,3,user_write);		// Set alarm time to DS1307's RAM
				_delay_cycles(1000000);
			}
		}
		_delay_cycles(200000);
	}
}

void LCD_Edit_Date_Time(void)
{
	timetype24h = 1;
	for (i=0;i<7;i++)
	{
		user_write[i] = dataread[i];
	}
	LCD_Write_Date_Time();
	user_set_digits[0] = dataread[3] / 16;
	user_set_digits[1] = dataread[3] % 16;
	user_set_digits[2] = dataread[2] / 16;
	user_set_digits[3] = dataread[2] % 16;
	user_set_digits[4] = dataread[1] / 16;
	user_set_digits[5] = dataread[1] % 16;
	user_set_digits[6] = dataread[4] % 8;
	user_set_digits[7] = dataread[5] / 16;
	user_set_digits[8] = dataread[5] % 16;
	user_set_digits[9] = dataread[6] / 16;
	user_set_digits[10] = dataread[6] % 16;
	user_set_digits[11] = dataread[7] / 16;
	user_set_digits[12] = dataread[7] % 16;
	i = 0;		// Cursor's step (0 -> 12)
	while(in_sub_menu != 0)
	{
		// Read Keypad
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		TA0CCTL1 = CCIE;
		Read_Keypad();
		if (button == 'a')
		{
			TA0CCTL1 &= ~CCIE;
			in_sub_menu = 0;
		}
		// Edit time in LCD
		if (button == 'r')
		{
			if (i <= 5)
				lcd_gotoxy(cursor_x[i],0);
			if ((i > 5) & (i < 13))
				lcd_gotoxy(cursor_x[i],1);
			if (i == 13)
				i = 0;
			if (i != 6)
				lcd_putc(user_set_digits[i] + 0x30);
			if (i == 6)		// Day (Mon,Tue,...)
				lcd_puts(week_days[user_set_digits[6] - 1]);
			i++;
		}
		if (button == 'l')
		{
			if (i <= 5)
				lcd_gotoxy(cursor_x[i],0);
			if ((i > 5) & (i < 13))
				lcd_gotoxy(cursor_x[i],1);
			if (i != 6)
				lcd_putc(user_set_digits[i] + 0x30);
			if (i == 6)		// Day (Mon,Tue,...)
				lcd_puts(week_days[user_set_digits[6] - 1]);
			i--;
		}
		if (i <= 5)
			lcd_gotoxy(cursor_x[i],0);
		if ((i > 5) & (i < 13))
			lcd_gotoxy(cursor_x[i],1);
		if (i == 13)
			i = 0;
		if (button == 'u')
		{
			user_set_digits[i]++;
			if (user_set_digits[i] > digits_max[i])
				user_set_digits[i] = 0;
		}
		if (button == 'd')
		{
			if (user_set_digits[i] > 0)
				user_set_digits[i]--;
			if (user_set_digits[i] == 0)
				user_set_digits[i] = digits_max[i];
		}
		if (i != 6)
			lcd_putc(user_set_digits[i] + 0x30);
		if (i == 6)		// Day (Mon,Tue,...)
		{
			if (user_set_digits[6] == 7)
				user_set_digits[6] = 0;
			lcd_puts(week_days[user_set_digits[6] - 1]);
		}
		if (button == 'o')
		{
			TA0CCTL1 &= ~CCIE;
			// Re-calculate time
			user_write[0] = (user_set_digits[4] << 4) + user_set_digits[5];		// Second
			user_write[1] = (user_set_digits[2] << 4) + user_set_digits[3];		// Minute
			user_write[2] = (user_set_digits[0] << 4) + user_set_digits[1];		// Hour
			user_write[3] = user_set_digits[6];									// Day (Mon,Tue,...)
			user_write[4] = (user_set_digits[7] << 4) + user_set_digits[8];		// Date
			user_write[5] = (user_set_digits[9] << 4) + user_set_digits[10];	// Month
			user_write[6] = (user_set_digits[11] << 4) + user_set_digits[12];	// Year
			// Check new time
			check_time_ok = 1;
			if (user_write[2] > 0x23)	// Hour <=23
			{
				lcd_gotoxy(0,0);
				lcd_puts("Hour is 0h->23h ");
				lcd_gotoxy(0,1);
				lcd_puts("Please re-check ");
				check_time_ok = 0;
				user_write[2] = 0x23;
				_delay_cycles(1000000);
			}
			if (d > 0x31)	// Date <= 31
			{
				lcd_gotoxy(0,0);
				lcd_puts(" Day is 01->31  ");
				lcd_gotoxy(0,1);
				lcd_puts("Please re-check ");
				check_time_ok = 0;
				user_write[4] = 0x31;
				_delay_cycles(1000000);
			}
			if (m > 0x12)	// Month <= 12
			{
				lcd_gotoxy(0,0);
				lcd_puts("Month is 01->12 ");
				lcd_gotoxy(0,1);
				lcd_puts("Please re-check ");
				check_time_ok = 0;
				user_write[5] = 0x12;
				_delay_cycles(1000000);
			}
			// Month 4,6,9,11 have 30 days (defined m as user_write[5])
			if (((m == 0x04)||(m == 0x06)||(m == 0x09)||(m == 0x11)||(m == 0x12)) & (user_write[4] == 0x31))
			{
				lcd_gotoxy(0,0);
				lcd_puts(" Month ");
				lcd_putc(user_set_digits[9] + 0x30);
				lcd_putc(user_set_digits[10] + 0x30);
				lcd_puts(" has   ");
				lcd_gotoxy(0,1);
				lcd_puts(" only 30 days!  ");
				check_time_ok = 0;
				_delay_cycles(1500000);
			}
			// Not leap year, Feb has 28 days
			if ((m == 0x02) & (((user_set_digits[11]*10 + user_set_digits[12]) % 4)!=0) & (d > 0x28) & (d < 0x32))
			{
				lcd_gotoxy(0,0);
				lcd_puts(" Feb 20");
				lcd_putc(user_set_digits[11] + 0x30);
				lcd_putc(user_set_digits[12] + 0x30);
				lcd_puts(" has   ");
				lcd_gotoxy(0,1);
				lcd_puts(" only 28 days!  ");
				check_time_ok = 0;
				_delay_cycles(1500000);
			}
			// Leap year, Feb has 29 days
			if ((m == 0x02) & (((user_set_digits[11]*10 + user_set_digits[12]) % 4)==0) & (d > 0x29) & (d < 0x32))
			{
				lcd_gotoxy(0,0);
				lcd_puts(" Feb 20");
				lcd_putc(user_set_digits[11] + 0x30);
				lcd_putc(user_set_digits[12] + 0x30);
				lcd_puts(" has   ");
				lcd_gotoxy(0,1);
				lcd_puts(" only 29 days!  ");
				check_time_ok = 0;
				_delay_cycles(1500000);
			}
			if (check_time_ok == 0)
				in_sub_menu = 0;
			// If check is ok, set new time
			if (check_time_ok == 1)
			{
				i2c_write(0x00,7,user_write);			// Set time for DS1307
				lcd_gotoxy(0,0);
				lcd_puts("Date&Time Reset!");
				lcd_gotoxy(0,1);
				lcd_puts("----------------");
				in_sub_menu = 0;
				_delay_cycles(1000000);
			}
		}
		_delay_cycles(200000);
	}
}

void LCD_Setting(void)
{
	while(in_sub_menu != 0)
	{
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		// Sub menu list show
		if ((0 < sub_menu_select) & (sub_menu_select <= 2))
		{
			lcd_gotoxy(1,0);
			lcd_puts("1. 24h/12h: ");
			if (timetype24h == 1)
				lcd_puts("24h");
			else
				lcd_puts("12h");
			lcd_gotoxy(1,1);
			lcd_puts("2. Language ENG");
		}
		if ((2 < sub_menu_select) & (sub_menu_select <= 4))
		{
			lcd_gotoxy(1,0);
			lcd_puts("3.Recalib Temp ");
			lcd_gotoxy(1,1);
			lcd_puts("4.Recalib Light");
		}
		if (sub_menu_select > 4)
		{
			lcd_gotoxy(1,0);
			lcd_puts("5. Recalib     ");
			lcd_gotoxy(1,1);
			lcd_puts("   Keypad      ");
		}
		// Read Keypad
		Read_Keypad();
		if (button == 'a')
			in_sub_menu = 0;
		if ((button == 'd') & (sub_menu_select < 6))
			sub_menu_select++;
		if ((button == 'u') & (sub_menu_select > 0))
			sub_menu_select--;
		if (sub_menu_select == 0)
			sub_menu_select = 5;
		if (sub_menu_select == 6)
			sub_menu_select = 1;
		// Sub menu move
		if ((sub_menu_select % 2) == 1)
		{
			lcd_gotoxy(0,0);
			lcd_putc(126);
			lcd_gotoxy(0,1);
			lcd_putc(' ');
		}
		else
		{
			lcd_gotoxy(0,0);
			lcd_putc(' ');
			lcd_gotoxy(0,1);
			lcd_putc(126);
		}
		// Press OK to choose a item
		if (button == 'o')
		{
			switch (sub_menu_select)
			{
				case 1:					// AM/PM
					lcd_gotoxy(13,0);
					// Small switch: 12h and 24h
					switch (timetype24h)
					{
						case 1:
						{
							timetype24h = 0;
							lcd_puts("12");
							break;
						}
						case 0:
						{
							timetype24h = 1;
							lcd_puts("24");
							break;
						}
					}
					_delay_cycles(300000);
					// End of small switch
					break;
				case 2:					// Language
					lcd_clear();
					lcd_puts("Only English now");
					_delay_cycles(1000000);
					break;
				case 3:					// Recalib Temp
					lcd_clear();
					lcd_puts("Coming soon");
					_delay_cycles(1000000);
					break;
				case 4:					// Recalib Light
					lcd_clear();
					lcd_puts("Coming soon");
					_delay_cycles(1000000);
					break;
				case 5:					// Recalib Keypad
					lcd_clear();
					lcd_puts("Coming soon");
					_delay_cycles(1000000);
					break;
			}
		}
		_delay_cycles(50000);
	}
}

void LCD_Hardware_Test(void)
{
	lcd_gotoxy(0,0);
	lcd_puts(" Hardware test  ");
	lcd_gotoxy(0,1);
	lcd_puts("will start in 3s");
	_delay_cycles(1000000);
	lcd_gotoxy(14,1);
	lcd_putc('2');
	_delay_cycles(1000000);
	lcd_gotoxy(14,1);
	lcd_putc('1');
	_delay_cycles(1000000);
	TA0CCTL0 &= ~CCIE;
	while(in_sub_menu != 0)
	{
		char i;
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		Read_Keypad();
		if (button == 'a')
			in_sub_menu = 0;	// Back to main menu
		lcd_gotoxy(0,0);
		lcd_puts("  Hold Cancle   ");
		lcd_gotoxy(0,1);
		lcd_puts("  to end test   ");
		for (i=0;i<8;i++)
		{
			write(led7large_test[i],led7large_test[i],led7small_test[i],led7small_test[i],led7small_test[i],led7small_test[i],0,0);
			write(led7large_test[i],led7large_test[i],led7small_test[i],led7small_test[i],led7small_test[i],led7small_test[i],0,0);
			pulse(STCP);			// 16 bits go out
			_delay_cycles(500000);
		}
		for (i=1;i<=2;i++)
		{
			write(0,0,0,0,0,0,0,0);
			write(0,0,0,0,0,0,0,0);
			pulse(STCP);			// 16 bits go out
			_delay_cycles(500000);
			write(255,255,255,255,255,255,255,255);
			write(255,255,255,255,255,255,255,255);
			pulse(STCP);			// 16 bits go out
			_delay_cycles(500000);
		}
	}
	TA0CCTL0 = CCIE;
}

void LCD_About(void)
{
	while(in_sub_menu != 0)
	{
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		if (in_sub_menu == 50)
		{
			lcd_gotoxy(0,0);
			lcd_puts(" N-CLOCK v1.0  ");
			lcd_gotoxy(0,1);
			lcd_puts("Last code: 08/02");
			lcd_gotoxy(15,0);
			lcd_putc(126);
		}
		if (in_sub_menu == 51)
		{
			lcd_gotoxy(0,1);
			lcd_putc(127);
			lcd_gotoxy(0,0);
			lcd_puts("  With my heart ");
			lcd_gotoxy(1,1);
			lcd_puts(" Not for resale");
		}
		Read_Keypad();
		if (button == 'a')
			in_sub_menu = 0;	// Back to main menu
		if (button == 'r')
			in_sub_menu = 51;	// Next info
		if (button == 'l')
			in_sub_menu = 50;	// Back info
	}
}

void LCD_Contact(void)
{
	while(in_sub_menu != 0)
	{
		ADC10CTL0 |= ENC + ADC10SC;			//Enable ADC10, Start sample - conversion
		_bis_SR_register(LPM0_bits + GIE);	//Enter LPM0, wait for sample-conversion finish
		if (in_sub_menu == 60)
		{
			lcd_gotoxy(0,0);
			lcd_puts(" Le Tri Thong  ");
			lcd_gotoxy(0,1);
			lcd_puts("Y!M kisseshacker");
			lcd_gotoxy(15,0);
			lcd_putc(126);
		}
		if (in_sub_menu == 61)
		{
			lcd_gotoxy(0,1);
			lcd_putc(127);
			lcd_gotoxy(0,0);
			lcd_puts(" Pay It Forward ");
			lcd_gotoxy(1,1);
			lcd_puts("  Club - DHBK  ");
		}
		Read_Keypad();
		if (button == 'a')
			in_sub_menu = 0;	// Back to main menu
		if (button == 'r')
			in_sub_menu = 61;	// Next info
		if (button == 'l')
			in_sub_menu = 60;	// Back info
	}
}

void LCD_Write_Date_Time(void)
{
	lcd_gotoxy(0,0);
	lcd_puts("    ");
	lcd_putc((dataread[3] / 16) % 4 + 0x30);	// 2 bits high of hour
	lcd_putc(dataread[3] % 16 + 0x30);			// 4 bits low of hour
	lcd_putc(':');
	lcd_putc(dataread[2] / 16 + 0x30);			// 4 bits high of minute
	lcd_putc(dataread[2] % 16 + 0x30);			// 4 bits low of minute
	lcd_putc(':');
	lcd_putc(dataread[1] / 16 + 0x30);			// 4 bits high of second
	lcd_putc(dataread[1] % 16 + 0x30);			// 4 bits low of second
	lcd_puts("    ");
	lcd_gotoxy(0,1);
	lcd_puts(week_days[dataread[4] % 8 - 1]);	// 3 bits low of day (Mon, Tue, Wed,...)
	lcd_puts(", ");
	lcd_putc((dataread[5] / 16) % 4 + 0x30);	// 2 bits high of date
	lcd_putc(dataread[5] % 16 + 0x30);			// 4 bits low of date
	lcd_putc('/');
	lcd_putc((dataread[6] / 16) % 2 + 0x30);	// 1 bit high of month
	lcd_putc(dataread[6] % 16 + 0x30);			// 4 bits low of month
	lcd_puts("/20");
	lcd_putc((dataread[7] / 16) % 16 + 0x30);	// 4 bit high of year
	lcd_putc(dataread[7] % 16 + 0x30);			// 4 bits low of year
	lcd_putc(' ');
}

void LCD_Write_Keypad(char col, char row)
{
	lcd_gotoxy(col,row);
	lcd_putc((volt0 / 100) + 48);				//ASCII code (of a number) = number + 48
	lcd_puts(".");
	lcd_putc((volt0 / 10) % 10 + 48);
	lcd_putc((volt0 % 10) + 48);
	lcd_puts("V");
	lcd_puts(" -         ");
	lcd_gotoxy(col+8,row);
	if (button == 0)	// 189 - 190
	{
		lcd_puts("NO KEY  ");
	}
	if (button == 'r')	// 189 - 190
	{
		lcd_puts("RIGHT   ");
	}
	if (button == 'l')	// 203 - 207
	{
		lcd_puts("LEFT    ");
	}
	if (button == 'u')	// 223 - 224
	{
		lcd_puts("UP      ");
	}
	if (button == 'd')	// 248 - 250
	{
		lcd_puts("DOWN    ");
	}
	if (button == 'o')	// 278 - 279
	{
		lcd_puts("OK      ");
	}
	if (button == 'a')	// 313
	{
		lcd_puts("AL");
	}
}

void LCD_Write_LM35(char col,char row)
{
	lcd_gotoxy(col,row);
	lcd_putc((volt1 / 100) + 48);				//ASCII code (of a number) = number + 48
	lcd_putc((volt1 / 10) % 10 + 48);
	lcd_putc((volt1 % 10) + 48);
	lcd_puts("*C ");
}

void LCD_Write_LDR(char col,char row)
{
	lcd_putc((volt3 / 100) + 48);				//ASCII code (of a number) = number + 48
	lcd_putc((volt3 / 10) % 10 + 48);
	lcd_putc((volt3 % 10) + 48);
	lcd_puts("dark");
	lcd_gotoxy(col,row);
}

/********************** READ ADC VALUES **************************/

void Read_Keypad(void)
{
	volt0 = (adc_result0 * Vref * 100) / 1024;	//Calculate result (= 100 * Voltage)
	// Compare ADC value (these numbers are ADC value range of each button)
	if (volt0 < 170)
		button = 0;
	if ((185 < volt0) & (volt0 < 195))	// 189 - 190
		button = 'r';
	if ((200 < volt0) & (volt0 < 210))	// 203 - 207
		button = 'l';
	if ((220 < volt0) & (volt0 < 230))	// 223 - 224
		button = 'u';
	if ((240 < volt0) & (volt0 < 255))	// 248 - 250
		button = 'd';
	if ((270 < volt0) & (volt0 < 285))	// 278 - 279
		button = 'o';
	if (volt0 > 310)					// 313
		button = 'a';
}

void Read_LM35(void)
{
	volt1 = (adc_result1 * Vref * 100) / 1024;	//Calculate result (= 100 * Voltage)
}

void Read_LDR(void)
{
	volt3 = (adc_result3 * Vref * 100) / 1024;	//Calculate result (= 100 * Voltage)
}

/*************************** INIT ********************************/

void Port_Init(void)
{
	P1SEL = 0;
	P1SEL2 = 0;
	P2SEL = 0;
	P2SEL2 = 0;
	P3SEL = 0;
	P3SEL2 = 0;

	P1DIR = 0xFF;
	P1OUT &= ~BIT2;		// Buzzer off
	P2DIR = 0xFF;
	P3DIR = 0xFF;
}

void Timer_Init(void)
{
	TA0CTL = TASSEL_2 + ID_3 + MC_1 + TAIE; // SMCLK (1Mhz), Continous mode, Devided by 8 (8us)
	TA0CCR0 = 62500;
	TA0CCR1 = 2000;
	TA0CCTL0 = CCIE;
//	TA0CCTL1 = CCIE;
}

void ADC10_Init(void)
{
	ADC10CTL0 = SREF_0 + ADC10SHT_0 + ADC10ON + ADC10IE + MSC;
	ADC10CTL1 = ADC10DIV_0 + ADC10SSEL_3 + CONSEQ_0;
	ADC10AE0 |= BIT0 + BIT1 + BIT3;
}

/*****************************************************************
* INTERRUPT PROGRAM
******************************************************************/

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_Interrupt(void)
{
//	ADC10CTL0 &= ~ENC;                      // ADC10 disable
//	switch (ADC_select)
//	{
//		case 0:
//			adc_result0 = ADC10MEM;					//Save Result
//			ADC10CTL1 = INCH_1;
//			ADC_select = 1;
//			break;
//		case 1:
//			adc_result1 = ADC10MEM;					//Save Result
//			ADC10CTL1 = INCH_3;
//			ADC_select = 3;
//			break;
//		case 3:
			adc_result0 = ADC10MEM;					//Save Result
			ADC10CTL1 = INCH_0;
//			ADC_select = 0;
//			break;
//	}
//	ADC10CTL0 |= ENC;
	_bic_SR_register_on_exit(LPM0_bits);	//Exit LPM0
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void CCR0_Interrupt(void)
{
	Calculate_Time();
	Check_Alarm();
	write(hourL,minuteL,dateL,monthL,yearL,tempL,led1,led2);			// 4 bits low
	write(hourH,minuteH,dateH,monthH,yearH,tempH,led1,led2);			// 4 bits high
	pulse(STCP);			// 16 bits go out
	if (alarm_beep == 1)
	{
		P1OUT |= BIT2;
		_delay_cycles(50000);
		P1OUT &= ~BIT2;
		_delay_cycles(50000);
		P1OUT |= BIT2;
		_delay_cycles(50000);
		P1OUT &= ~BIT2;
		_delay_cycles(50000);
	}
	_bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void CCR1_Interrupt(void)
{
	switch (TA0IV)
	{
		case 0x02:	// CCR1
		{
			if (in_sub_menu == 20)	// Editing Time
			{
				if (i <= 5)
					lcd_gotoxy(cursor_x[i],0);
				if ((i > 5) & (i < 13))
					lcd_gotoxy(cursor_x[i],1);
			}
			if (in_sub_menu == 10)	// Editing Alarm Time
				lcd_gotoxy(cursor_x[i],1);
			lcd_putc(' ');
			break;
		}
	}
	_bic_SR_register_on_exit(LPM0_bits);
}

/****************************************************************
 * END OF N_Clock_main.c
 ****************************************************************/
