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
	uint16_t addr = 0;
	char string[30] = " ";
	uint8_t y = 0;
	uint16_t addr1 = 0;
	uint16_t addr2 = 0;
	
	enum { nichts, scann, reset } zustand = nichts;
	
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
	
	while ( 1 ){
		
		if ( !(PIND & (1<<PD6)) ){
			zustand = scann;
		}
		if ( !(PIND & (1<<PD5)) ){
			zustand = reset;
		}
		
		switch ( zustand ){
			
			case nichts:
				
			break;
			case scann:
				
				glcd_clear();
				glcd_clear_buffer();
				
				sprintf(string,"addr.:");
				glcd_draw_string_xy(0,0,string);
				glcd_write();
				
				y = 0;
				addr = 0;
				addr1 = 0;
				addr2 = 0;
				
				while( addr < 0xFF ){
				
					addr1++;
					
					if( addr1 == 0x0F ){
						addr2++;
						addr1 = 0;
					}
					
					addr = ( addr1 + ( addr2 << 4 ));
					
					TWIStart();
					TWIWrite ( addr );
					
					
					
					if ( TWIGetStatus() == 0x18 ){
						sprintf(string,"0x%x", addr);
						glcd_draw_string_xy(40,y,string);
						glcd_write();
						y += 8;
					}
					
					if( addr > 0xFF ){
						sprintf(string,"FINISH        ");
						glcd_draw_string_xy(0,40,string);
						glcd_write();
					}
					
					else{
						sprintf(string,"act addr.:0x%x", addr);
						glcd_draw_string_xy(0,40,string);
						glcd_write();
					}
					
					TWIStop();
					
				}
				
				zustand = nichts;
				
			break;
			case reset:
				
				glcd_clear();
				glcd_write();
				glcd_tiny_set_font(Font5x7,5,7,32,127);
				glcd_clear_buffer();
				
				
			break;
			
		}// end of switch
	}// end of while
	return 0;
}//end of main
