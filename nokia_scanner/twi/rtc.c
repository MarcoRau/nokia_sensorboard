
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "rtc.h"

#define RTCREAD 	0b11011111
#define RTCWRITE	0b11011110


void InitRTC( void ){
	RTCWriteByte(0x00, 0b10000000);
	_delay_ms(50);
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