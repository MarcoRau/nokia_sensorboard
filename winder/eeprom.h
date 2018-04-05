
//#include "twi.c"
//#include "eeprom.c"

void TWIInit	( void );		// Initial TWI
void TWIStart	( void );		// Start read/write
void TWIStop	( void );		// Stop  read/write
void TWIWrite	( uint8_t );	// Write

uint8_t TWIReadACK		( void );
uint8_t TWIReadNACK		( void );
uint8_t TWIGetStatus	( void );

uint8_t EEWrite1Byte	( uint16_t, uint8_t );
uint8_t EERead1Byte	 	( uint16_t );

uint8_t  EEWrite2Bytes	( uint16_t, uint16_t );
uint16_t EERead2Bytes	( uint16_t );