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

#define F_CPU 16000000UL  // 1 MHz

#define RTCREAD  0b11011111
#define RTCWRITE 0b11011110

#define EERTCREAD  0b10101111
#define EERTCWRITE 0b10101110

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
	
	uint8_t choice = 1;
	
	enum state{ off, menu, settings, act_time } state = off;
	
	uint8_t time	= 90;
	
	uint8_t circle 	= 0;
	
	uint8_t sec_old = 0;
	
	// Time
	
	uint8_t sec_one = 0;
	uint8_t sec_ten = 0;
	uint8_t min_one = 0;
	uint8_t min_ten = 0;
	uint8_t h_one   = 0;
	uint8_t h_ten   = 0;
	
	// Date
	
	uint8_t week		= 1;
	uint8_t day_one		= 1;
	uint8_t day_ten		= 0;
	uint8_t month_one	= 1;
	uint8_t month_ten	= 0;
	uint8_t year_one	= 0;
	uint8_t year_ten	= 0;
	
	/* Backlight pin PL3, set as output, set high for 100% output */
	DDRB |= (1<<PB2);
	//PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
	DDRC &= ~(1<<PC0); //Eingang Hallsensor
	PORTC |= (1<<PC0);	//Pullup Hallsensor einschalten
	
	DDRC |=(1<<PC1); 	//Eingang Hallsensor
	PORTC |= (1<<PC1);	//Pullup Hallsensor einschalten
	
	
	DDRD &= ~((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//Taster 1-3
	PORTD |= ((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//PUllups für Taster einschalten
	
	DDRD &= ~(1<<PD4); //T0 Counter Input
	TCCR0B |= (1<<CS02) | (1<<CS01) | (1<<CS00);//Counter 0 enabled clock on rising edge
	
	//Timer 1 Configuration
	OCR1A = 0x009C;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

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
	
	uint8_t RTCReadByte ( uint8_t u8addr ){
		
		uint8_t u8data = 0;
		
		TWIStart();
		TWIWrite( RTCWRITE );
		TWIWrite( u8addr );
		TWIStart();
		TWIWrite( RTCREAD );
		u8data = TWIReadNACK();
		TWIStop();
		
		return u8data;
	}
	
	uint8_t RTCWriteByte ( uint8_t u8addr, uint8_t u8data ){
		
		TWIStart();
		TWIWrite( RTCWRITE );  
		TWIWrite( u8addr );
		TWIWrite( u8data );
		TWIStop();
		
		_delay_ms(50);
		
		return 0;
	}
	
	void InitRTC( void ){
		RTCWriteByte(0x00, 0b10000000);
		_delay_ms(50);
	}
	
	InitRTC();
	
	while( 1 ){
		if(!(PIND & (1<<PD6))){
			choice--;
			if( choice == 0 ){
				choice = 3;
			}
			_delay_ms(time);
			glcd_clear();
			glcd_clear_buffer();
		}
		
		if(!(PIND & (1<<PD5))){
			choice++;
			if( choice == 4 ){
				choice = 1;
			}
			_delay_ms(time);
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
		sprintf(string,"Settings");
		glcd_draw_string_xy(0,16,string);
		glcd_write();
		sprintf(string,"<");
		
		// Auswahl von Inhaltsverzeichnis
		
		switch ( choice ){
			case 1:
				glcd_draw_string_xy(78,0,string);
				if(!(PIND & (1<<PD2))){
					state = off;
					_delay_ms(time);
				}
			break;
			case 2:
				glcd_draw_string_xy(78,8,string);
				if(!(PIND & (1<<PD2))){
					state = act_time;
					_delay_ms(time);
				}
			break;
			case 3:
				glcd_draw_string_xy(78,16,string);
				if(!(PIND & (1<<PD2))){
					state = settings;
					_delay_ms(time);
				}
			break;
		}
		
		glcd_write();
		
		// Modus
		
		switch( state ){
			
			case menu:
			
			break;
			case off:
				glcd_clear();
				glcd_write();
				glcd_clear_buffer();
				while((PIND & (1<<PD2)));
				state = menu;
				_delay_ms(time);
			break;
			case settings:
				
				// Date
				
				glcd_clear();
				glcd_clear_buffer();
				sprintf(string,"Date:");
				glcd_draw_string_xy(0,0,string);
				for( circle = 0; circle < 1; ){		// Year
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						year_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						year_one++;
						_delay_ms(100);
					}
					if( year_one == 10 ){
						year_one = 0;
						year_ten++;
						if( year_ten == 10 ){
							year_ten = 0;
						}
					}
					if( year_one == 255 ){
						year_one = 9;
						year_ten--;
						if( year_ten == 255 ){
							year_ten = 9;
						}
					}
					sprintf(string,"%d%d", year_ten, year_one);
					glcd_draw_string_xy(64,0,string);
					glcd_write();
				}
				RTCWriteByte(0x06, (( year_ten << 4 ) + year_one ));
				_delay_ms(time);
				for( circle = 0; circle < 1; ){		// Month
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						month_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						month_one++;
						_delay_ms(100);
					}
					if( month_one == 10 ){
						month_one = 0;
						month_ten++;
						if( month_ten == 4 ){
							month_ten = 0;
						}
					}
					if( month_one == 255 ){
						month_one = 9;
						month_ten--;
						if( month_ten == 255 ){
							month_ten = 1;
						}
					}
					if(( month_ten == 1 ) && ( month_one == 3 )){
						month_ten	= 0;
						month_one	= 1;
					}
					if(( month_ten == 1 ) && ( month_one == 9 )){
						month_ten	= 1;
						month_one	= 2;
					}
					sprintf(string,"%d%d.", month_ten, month_one);
					glcd_draw_string_xy(47,0,string);
					glcd_write();
				}
				RTCWriteByte(0x05, (( month_ten << 4 ) + month_one ));
				_delay_ms(time);
				for( circle = 0; circle < 1; ){		// Day
				
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						day_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						day_one++;
						_delay_ms(100);
					}
					if( day_one == 10 ){
						day_one = 0;
						day_ten++;
						if( day_ten == 4 ){
							day_ten = 0;
						}
					}
					if( day_one == 255 ){
						day_one = 9;
						day_ten--;
						if( day_ten == 255 ){
							day_ten = 3;
						}
					}
					if(( day_ten == 3 ) && ( day_one == 3 )){
						day_ten	= 0;
						day_one	= 0;
					}
					if(( day_ten == 3 ) && ( day_one == 9 )){
						day_one	= 2;
					}
					sprintf(string,"%d%d.", day_ten, day_one);
					glcd_draw_string_xy(30,0,string);
					glcd_write();
				}
				RTCWriteByte(0x04, (( day_ten << 4 ) + day_one ));
				_delay_ms(time);
				for( circle = 0; circle < 1; ){		// Weekday
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						week--;
						if( week == 0 ){
							week = 7;
						}
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						week++;
						if( week == 8 ){
							week = 0;
						}
						_delay_ms(100);
					}
					sprintf(string,"day: %d", week);
					glcd_draw_string_xy(0,16,string);
					glcd_write();
				}
				RTCWriteByte(0x03, week );
				_delay_ms(time);
				
				// Time
				
				glcd_clear();
				glcd_clear_buffer();
				for( circle = 0; circle < 1; ){		// Houer
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						h_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						h_one++;
						_delay_ms(100);
					}
					if( h_one == 10 ){
						h_one = 0;
						h_ten++;
						if( h_ten == 3 ){
							h_ten = 0;
						}
					}
					if( h_one == 255 ){
						h_one = 9;
						h_ten--;
						if( h_ten == 255 ){
							h_ten = 2;
						}
					}
					if(( h_ten == 2 ) && ( h_one == 4 )){
						h_ten	= 0;
						h_one	= 0;
					}
					if(( h_ten == 2 ) && ( h_one == 9 )){
						h_ten	= 2;
						h_one	= 3;
					}
					sprintf(string,"h:   %d%d", h_ten, h_one);
					glcd_draw_string_xy(0,0,string);
					glcd_write();
				}
				RTCWriteByte(0x02, (( h_ten << 4 ) + h_one ));
				_delay_ms(time);
				for( circle = 0; circle < 1; ){		// Minute
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						min_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						min_one++;
						_delay_ms(100);
					}
					if( min_one == 10 ){
						min_one = 0;
						min_ten++;
						if( min_ten == 6 ){
							min_ten = 0;
						}
					}
					if( min_one == 255 ){
						min_one = 9;
						min_ten--;
						if( min_ten == 255 ){
							min_ten = 5;
						}
					}
					sprintf(string,"min: %d%d", min_ten, min_one);
					glcd_draw_string_xy(0,8,string);
					glcd_write();
				}
				RTCWriteByte(0x01, (( min_ten << 4 ) + min_one ));
				_delay_ms(time);
				for( circle = 0; circle < 1; ){		// Second
					if(!(PIND & (1<<PD2))){
						circle = 2;
					}
					if(!(PIND & (1<<PD6))){
						sec_one--;
						_delay_ms(100);
					}
					if(!(PIND & (1<<PD5))){
						sec_one++;
						_delay_ms(100);
					}
					if( sec_one == 10 ){
						sec_one = 0;
						sec_ten++;
						if( sec_ten == 6 ){
							sec_ten = 0;
						}
					}
					if( sec_one == 255 ){
						sec_one = 9;
						sec_ten--;
						if( sec_ten == 255 ){
							sec_ten = 5;
						}
					}
					sprintf(string,"sec: %d%d", sec_ten, sec_one);
					glcd_draw_string_xy(0,16,string);
					glcd_write();
				}
				RTCWriteByte(0x00, (( sec_ten << 4 ) + sec_one + 0x80 ));
				_delay_ms(time);
				glcd_clear();
				glcd_clear_buffer();
				state = menu;	
			break;
			case act_time:
				glcd_clear();
				glcd_clear_buffer();
				for( circle = 0; circle < 1; ){
					
					sec_one	= (RTCReadByte(0x00) & 0b00001111 );
					sec_ten	= (RTCReadByte(0x00) & 0b01110000 ) >> 4;
					min_one	= (RTCReadByte(0x01) & 0b00001111 );
					min_ten	= (RTCReadByte(0x01) & 0b01110000 ) >> 4;
					h_one	= (RTCReadByte(0x02) & 0b00001111 );
					h_ten	= (RTCReadByte(0x02) & 0b00110000 ) >> 4;
					
					week		= (RTCReadByte(0x03) & 0b00000111 );
					day_one		= (RTCReadByte(0x04) & 0b00001111 );
					day_ten		= (RTCReadByte(0x04) & 0b00110000 ) >> 4;
					month_one	= (RTCReadByte(0x05) & 0b00001111 );
					month_ten	= (RTCReadByte(0x05) & 0b00110000 ) >> 4;
					year_one	= (RTCReadByte(0x06) & 0b00001111 );
					year_ten	= (RTCReadByte(0x06) & 0b00110000 ) >> 4;
					
					if( sec_one != sec_old ){
						
						glcd_clear();
						glcd_clear_buffer();
						
						switch ( week ){
							case 1:
								sprintf(string,"Monday");
							break;
							case 2:
								sprintf(string,"Tuesday");
							break;
							case 3:
								sprintf(string,"Wednesday");
							break;
							case 4:
								sprintf(string,"Thursday");
							break;
							case 5:
								sprintf(string,"Friday");
							break;
							case 6:
								sprintf(string,"Saturday");
							break;
							case 7:
								sprintf(string,"Sunday");
							break;
						}
						glcd_draw_string_xy(0,0,string);
						glcd_write();
						
						sprintf(string,"%d%d.%d%d.%d%d ", day_ten , day_one, month_ten, month_one, year_ten, year_one);
						glcd_draw_string_xy(0,16,string);
						glcd_write();
						
						sprintf(string,"%d%d:%d%d:%d%d ", h_ten , h_one, min_ten, min_one, sec_ten, sec_one);
						glcd_draw_string_xy(0,32,string);
						glcd_write();
					}
					sec_old = sec_one;
					if(!(PIND & (1<<PD2))){
						circle = 2;
						_delay_ms(200);
					}
				}
				_delay_ms(time);
				state = menu;
				glcd_clear();
				glcd_clear_buffer();
				glcd_write();
			break;
		}
	}
	return 0;
}//end of main
