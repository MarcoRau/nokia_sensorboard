
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

// Initial TWI
void TWIInit ( void ){
	// set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	// enable TWI
	TWCR = ( 1 << TWEN );
}
//
// Start read/write
void TWIStart ( void ){

	TWCR = ( 1 << TWINT ) | ( 1 << TWSTA ) | ( 1 << TWEN );
	while (( TWCR & ( 1 << TWINT )) == 0 );
}
//
// Stop read/ write
void TWIStop ( void ){

	TWCR = ( 1 << TWINT ) | ( 1 << TWSTO ) | ( 1 << TWEN );
}
//
// Write
void TWIWrite ( uint8_t u8data ){

	TWDR = u8data;
	TWCR = ( 1 << TWINT ) | ( 1 << TWEN );
	while (( TWCR & ( 1 << TWINT )) == 0 );
}
//
// Read
uint8_t TWIReadACK ( void ){

	TWCR = ( 1 << TWINT ) | ( 1 << TWEN )|( 1 << TWEA );
	while (( TWCR & ( 1 << TWINT )) == 0 );
	return TWDR;
}
//
// Read function
uint8_t TWIReadNACK ( void ){

	TWCR = ( 1 << TWINT ) | ( 1 << TWEN );
	while (( TWCR & ( 1 << TWINT )) == 0 );
	return TWDR;
}
//
// Status
uint8_t TWIGetStatus( void ){

	uint8_t status;
	// mask status
	status = TWSR & 0xF8;
	return status;
}