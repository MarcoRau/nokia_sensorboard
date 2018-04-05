//-------------------------------------------------------------
//	
//	file: eeprom.c
//	name: RAUM
//	date: 29.03.2018
// 
//	description:
//	These functions are for the eeprom on the nokia sensorboard.
//-------------------------------------------------------------

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "twi/twi.h"

#define EEWRITE		0b10100000 
#define EEREAD		0b10100001 

// 8 Bit data
void EEWrite1Byte ( uint16_t u16addr, uint8_t u8data ){

	uint8_t addr_l, addr_h;
	addr_l = u16addr;
	addr_h = ( u16addr >> 8 );
	
    TWIStart();
	TWIWrite( EEWRITE );  
	TWIWrite( addr_l );
	TWIWrite( addr_h );
	TWIWrite( u8data );
	TWIStop();
}


uint8_t EERead1Byte ( uint16_t u16addr ){

	uint8_t addr_l, addr_h;
	addr_l = u16addr;
	addr_h = ( u16addr >> 8 );
	uint8_t u8data = 0;
	
    TWIStart();
	TWIWrite( EEWRITE );
	TWIWrite( addr_l );
	TWIWrite(addr_h);
    TWIStart();
	TWIWrite( EEREAD );
	u8data = TWIReadNACK();
    TWIStop();
	
    return u8data;
}
//
// 16 Bit data
void  EEWrite2Bytes ( uint16_t u16addr, uint16_t u16data ){
	
	uint8_t data_lsb, data_msb;
	//uint8_t time = 50;
	
	data_lsb = u16data;							// LSB data
	data_msb = ( u16data >> 8 );				// MSB data
	
	EEWrite1Byte ( u16addr, data_lsb );			// write LSB register
	delay_ms(50);								// for write
	EEWrite1Byte (( u16addr + 1 ),  data_msb );// Write MSB register
	delay_ms(50);								// for write
}

uint16_t EERead2Bytes ( uint16_t u16addr ){
	
	uint16_t data_lsb, data_msb;
	
	data_lsb = EERead1Byte ( u16addr );			// read LSB register
	data_msb = EERead1Byte ( u16addr + 0x01 );	// read MSB register
	
    return ( data_lsb + ( data_msb << 8 ));	// 16 Bit
}