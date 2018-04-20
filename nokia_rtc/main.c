/*

	Demo of glcd library with AVR8 microcontroller
	
	Tested on a custom made PCB (intended for another project)

	See ../README.md for connection details

*/

#include <avr/io.h>
#include "glcd/glcd.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "glcd/fonts/Liberation_Sans15x21_Numbers.h"
#include "glcd/fonts/font5x7.h"
#include <avr/pgmspace.h>

#include "twi/eeprom.h"
#include "twi/rtc.h"

#define F_CPU 16000000UL  // 1 MHz

/* Function prototypes */
static void setup(void);

static void setup(void)
{
	/* Set up glcd, also sets up SPI and relevent GPIO pins */
	glcd_init();
}

uint8_t ms, ms10,ms100,sec,min,entprell, state;

ISR (TIMER1_COMPA_vect)
{
	ms10++;
	
	if(entprell != 0){
		entprell--;
	}
	
	//10ms
	if(ms10 == 10){
		ms10 = 0;
		ms100++;
	}
	
	//100ms
    if(ms100 == 10){
		ms100 = 0;
		sec++;
	}
	
	//Minute
	if(sec == 10){
		sec = 0;
		min++;
		if(state == 11){
			state = 10;
		}
	}
}

int main(void)
{	
	char string[30] = " ";
	
	char choice = 1;
	
	enum state{ menu, off, time, alarm1, settings } state = off;
	
	uint8_t delay_time	= 95;
	uint8_t circle 		= 0;
	uint8_t sec_old 	= 0;
	
	//--- READ ---
	// Time
	uint8_t sec_one = 0;
	uint8_t sec_ten = 0;
	uint8_t min_one = 0;
	uint8_t min_ten = 0;
	uint8_t h_one   = 0;
	uint8_t h_ten   = 0;
	// Date
	uint8_t week		= 1;
	uint8_t date_one	= 1;
	uint8_t date_ten	= 0;
	uint8_t month_one	= 1;
	uint8_t month_ten	= 0;
	uint8_t year_one	= 0;
	uint8_t year_ten	= 0;
	
	//--- WRITE ---
	// Time
	uint16_t year		= 0;
	uint8_t date		= 1;
	uint8_t month		= 1;
	uint8_t weekday		= 1;
	// Date
	uint8_t hour		= 0;
	uint8_t minute		= 0;
	uint8_t second		= 0;
	
	
	/* Backlight pin PL3, set as output, set high for 100% output */
	DDRB |= (1<<PB2);
	// PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
	DDRD &= ~((1<<PD6) | (1<<PD2) | (1<<PD5)); 	// Taster 1-3
	PORTD |= ((1<<PD6) | (1<<PD2) | (1<<PD5)); 	// PUllups für Taster einschalten
	
	DDRD &= ~(1<<PD4); 							// T0 Counter Input
	TCCR0B |= (1<<CS02) | (1<<CS01) | (1<<CS00);	// Counter 0 enabled clock on rising edge
	
	// Timer 1 Configuration
	OCR1A = 0x009C;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    // Set interrupt on compare match

    TCCR1B |= (1 << CS12) | (1 << CS10);
    // set prescaler to 1024 and start the timer

    sei();
    // enable interrupts
	
	setup();
	
	glcd_clear();
	glcd_write();
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_clear_buffer();
	
	min = 1;
	
	TWIInit();
	InitRTC();
	
	while( 1 ){
		switch( state ){
			case menu:				// -----> MENU <-----
				if(!(PIND & (1<<PD5))){
					choice++;
					if( choice == 5 ){
						choice = 1;
					}
					_delay_ms(delay_time);
					glcd_clear();
					glcd_clear_buffer();
				}
				if(!(PIND & (1<<PD6))){
					choice--;
					if( choice == 0 ){
						choice = 4;
					}
					_delay_ms(delay_time);
					glcd_clear();
					glcd_clear_buffer();
				}
				
				// Inhaltsverzeichnis
				sprintf(string,"OFF");
				glcd_draw_string_xy(0,0,string);
				glcd_write();
				sprintf(string,"Time");
				glcd_draw_string_xy(0,8,string);
				glcd_write();
				sprintf(string,"Alarm 1");
				glcd_draw_string_xy(0,16,string);
				glcd_write();
				sprintf(string,"Settings");
				glcd_draw_string_xy(0,24,string);
				glcd_write();
				
				// Auswahl von Inhaltsverzeichnis
				sprintf(string,"<");
				switch ( choice ){
					case 1: // off
						glcd_draw_string_xy(78,0,string);
						if(!(PIND & (1<<PD2))){
							state = off;
							_delay_ms(delay_time);
						}
					break;
					case 2: // time
						glcd_draw_string_xy(78,8,string);
						if(!(PIND & (1<<PD2))){
							state = time;
							_delay_ms(delay_time);
						}
					break;
					case 3: // alarm
						glcd_draw_string_xy(78,16,string);
						if(!(PIND & (1<<PD2))){
							state = alarm1;
							_delay_ms(delay_time);
						}
					break;
					case 4: // settings
						glcd_draw_string_xy(78,24,string);
						if(!(PIND & (1<<PD2))){
							state = settings;
							_delay_ms(delay_time);
						}
					break;
				}
				glcd_write();
			break;
			case off:				// -----> OFF <-----
				glcd_clear();
				glcd_clear_buffer();
				
				while((PIND & (1<<PD2)));
				
				state = menu;		// menu
				choice = 1; 		// off
				_delay_ms(delay_time);
				glcd_clear();
				glcd_clear_buffer();
			break;
			case time:				// -----> TIME <-----
				glcd_clear();
				glcd_clear_buffer();
				for( circle = 0; circle < 1; ){
					
					sec_one	= (RTCReadByte(0x00) & 0b00001111 );
					
					if( sec_one != sec_old ){
						
						sec_ten	= (RTCReadByte(0x00) & 0b01110000 ) >> 4;
						min_one	= (RTCReadByte(0x01) & 0b00001111 );
						min_ten	= (RTCReadByte(0x01) & 0b01110000 ) >> 4;
						h_one	= (RTCReadByte(0x02) & 0b00001111 );
						h_ten	= (RTCReadByte(0x02) & 0b00110000 ) >> 4;
						
						week		= (RTCReadByte(0x03) & 0b00000111 );
						date_one	= (RTCReadByte(0x04) & 0b00001111 );
						date_ten	= (RTCReadByte(0x04) & 0b00110000 ) >> 4;
						month_one	= (RTCReadByte(0x05) & 0b00001111 );
						month_ten	= (RTCReadByte(0x05) & 0b00110000 ) >> 4;
						year_one	= (RTCReadByte(0x06) & 0b00001111 );
						year_ten	= (RTCReadByte(0x06) & 0b00110000 ) >> 4;
						
						glcd_clear();
						glcd_clear_buffer();
						
						sprintf(string,"%d%d.%d%d.%d%d", date_ten , date_one, month_ten, month_one, year_ten, year_one);
						glcd_draw_string_xy(0,0,string);
						glcd_write();
						
						switch ( week ){
							case 1:
								sprintf(string,"Monday   ");
							break;
							case 2:
								sprintf(string,"Tuesday  ");
							break;
							case 3:
								sprintf(string,"Wednesday");
							break;
							case 4:
								sprintf(string,"Thursday ");
							break;
							case 5:
								sprintf(string,"Friday   ");
							break;
							case 6:
								sprintf(string,"Saturday ");
							break;
							case 7:
								sprintf(string,"Sunday   ");
							break;
						}
						glcd_draw_string_xy(0,16,string);
						glcd_write();
						
						sprintf(string,"%d%d:%d%d:%d%d ", h_ten , h_one, min_ten, min_one, sec_ten, sec_one);
						glcd_draw_string_xy(0,32,string);
						glcd_write();
					}
					sec_old = sec_one;
					if(!(PIND & (1<<PD2))){
						circle = 2;
						_delay_ms(delay_time);
					}
				}
				state = menu;		// menu
				choice = 1; 		// off
				
				glcd_clear();
				glcd_clear_buffer();
			break;
			case alarm1:			// -----> ALARM 1<-----
			break;
			case settings:			// -----> SETTINGS <-----
				glcd_clear();
				glcd_clear_buffer();
				
				sprintf(string,"Year   :" );
				glcd_draw_string_xy(0,0,string);
				sprintf(string,"Month  :" );
				glcd_draw_string_xy(0,8,string);
				sprintf(string,"Date   :" );
				glcd_draw_string_xy(0,16,string);
				sprintf(string,"Weekday:" );
				glcd_draw_string_xy(0,32,string);
				
				glcd_write();
				
				while ( PIND & (1<<PD2) ){			// Year
					if(!(PIND & (1<<PD6))){
						year--;
						if( year == 65535 ){			// Year down
							year = 99;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Year up
						year++;
						if( year == 100 ){
							year = 0;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", year );
					glcd_draw_string_xy(50,0,string);
					glcd_write();
				}
				RTCSetYear( year );
				_delay_ms(delay_time);
				while ( PIND & (1<<PD2) ){			// Month
					if(!(PIND & (1<<PD6))){
						month--;
						if( month == 0 ){			// Month down
							month = 12;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Month up
						month++;
						if( month == 13 ){
							month = 1;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", month );
					glcd_draw_string_xy(50,8,string);
					glcd_write();
				}
				RTCSetMonth( month );
				_delay_ms(delay_time);
				while ( PIND & (1<<PD2) ){			// Date
					if(!(PIND & (1<<PD6))){		// Date down
						date--;
						if( date == 0 ){
							date = 32;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Date up
						date++;
						if( date == 33 ){
							date = 1;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", date );
					glcd_draw_string_xy(50,16,string);
					glcd_write();
				}
				RTCSetDate( date );
				_delay_ms(delay_time);
				while ( PIND & (1<<PD2) ){			// Weekday
					switch ( weekday ){
						case 1:
							sprintf(string,"Mo");
						break;
						case 2:
							sprintf(string,"Tu");
						break;
						case 3:
							sprintf(string,"We");
						break;
						case 4:
							sprintf(string,"Th");
						break;
						case 5:
							sprintf(string,"Fr");
						break;
						case 6:
							sprintf(string,"Sa");
						break;
						case 7:
							sprintf(string,"Su");
						break;
					}
					if(!(PIND & (1<<PD6))){		// Weekday down
						weekday--;
						if( weekday == 0 ){
							weekday = 7;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Weekday up
						weekday++;
						if( weekday == 8 ){
							weekday = 1;
						}
						_delay_ms(delay_time);
					}
					glcd_draw_string_xy(50,32,string);
					glcd_write();
				}
				RTCSetWeekday( weekday );
				_delay_ms(delay_time);
				
				glcd_clear();
				glcd_clear_buffer();
				sprintf(string,"Hour   :" );
				glcd_draw_string_xy(0,0,string);
				sprintf(string,"Minute :" );
				glcd_draw_string_xy(0,8,string);
				sprintf(string,"Second :" );
				glcd_draw_string_xy(0,16,string);
				glcd_write();
				
				while ( PIND & (1<<PD2) ){			// Hour
					if(!(PIND & (1<<PD6))){		// Hour down
						hour--;
						if( hour == 255 ){
							hour = 24;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Hour up
						hour++;
						if( hour == 25 ){
							hour = 0;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", hour );
					glcd_draw_string_xy(50,0,string);
					glcd_write();
				}
				RTCSetHour( hour );
				_delay_ms(delay_time);
				while ( PIND & (1<<PD2) ){			// Minute
					if(!(PIND & (1<<PD6))){		// Minute down
						minute--;
						if( minute == 255 ){
							minute = 59;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Minute up
						minute++;
						if( minute == 60 ){
							minute = 0;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", minute );
					glcd_draw_string_xy(50,8,string);
					glcd_write();
				}
				RTCSetMinute( minute );
				_delay_ms(delay_time);
				while ( PIND & (1<<PD2) ){			// Second
					
					if(!(PIND & (1<<PD6))){		// Second down
						second--;
						if( second == 255 ){
							second = 59;
						}
						_delay_ms(delay_time);
					}
					if(!(PIND & (1<<PD5))){		// Second up
						second++;
						if( second == 60 ){
							second = 0;
						}
						_delay_ms(delay_time);
					}
					sprintf(string,"%d     ", second );
					glcd_draw_string_xy(50,16,string);
					glcd_write();
				}
				RTCSetSecond( second );
				_delay_ms(delay_time);
				state = menu;
				choice = 1;
				glcd_clear();
				glcd_clear_buffer();
			break;
		}// end of switch
	} // end of while
	return 0;
}//end of main
