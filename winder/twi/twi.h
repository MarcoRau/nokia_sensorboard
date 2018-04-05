
#include "twi/twi.c"

void TWIInit	( void );	// Initial TWI
void TWIStart	( void );	// Start read/write
void TWIStop	( void );	// Stop read/ write
void TWIWrite	( uint8_t );	// Write
uint8_t TWIReadACK		( void );
uint8_t TWIReadNACK		( void );
uint8_t TWIGetStatus	( void );