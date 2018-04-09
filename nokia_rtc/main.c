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
	uint8_t sec = 0;
	uint8_t min = 0;
	uint8_t h   = 0;
	uint8_t d   = 0;
	
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
	
	RTCWriteByte(0x00, 0b10000000);
	RTCWriteByte(0x07, 0x00);
	
	while( 1 ){
		
		sec	= RTCReadByte(0x00);
		min	= RTCReadByte(0x01);
		h	= RTCReadByte(0x02);
		d	= RTCReadByte(0x03);
		
		glcd_clear();
		glcd_clear_buffer();
		sprintf(string,"%d", sec);
		glcd_draw_string_xy(0,0,string);
		glcd_write();
		
		sprintf(string,"%d", min);
		glcd_draw_string_xy(0,8,string);
		glcd_write();
		
		sprintf(string,"%d", h);
		glcd_draw_string_xy(0,16,string);
		glcd_write();
		
		sprintf(string,"%d", d);
		glcd_draw_string_xy(0,24,string);
		glcd_write();
		
		
		_delay_ms(10);
	}
	
	
	return 0;
}//end of main
