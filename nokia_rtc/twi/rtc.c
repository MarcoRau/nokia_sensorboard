
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "rtc.h"

#define RTCREAD  0b11011111
#define RTCWRITE 0b11011110

void InitRTC( void ){
	RTCWriteByte(0x00, 0b10001000); // 0b10000000 --> Disable external input
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

// Set Time settings
//
void RTCSetYear( uint8_t year ){
	uint8_t	year_one = 0;
	uint8_t year_ten = 0;
	while ( year > 0){
		year_one ++;
		if( year_one == 10 ){
			year_one = 0;
			year_ten ++;
		}
		year --;
	}
	RTCWriteByte(0x06, (( year_ten << 4 ) + year_one ));
}
void RTCSetMonth( uint8_t month ){
	uint8_t	month_one = 0;
	uint8_t month_ten = 0;
	while ( month > 0){
		month_one ++;
		if( month_one == 10 ){
			month_one = 0;
			month_ten ++;
		}
		month --;
	}
	RTCWriteByte(0x05, (( month_ten << 4 ) + month_one ));
}
void RTCSetDate( uint8_t date ){
	uint8_t	date_one = 0;
	uint8_t date_ten = 0;
	while ( date > 0){
		date_one ++;
		if( date_one == 10 ){
			date_one = 0;
			date_ten ++;
		}
		date --;
	}
	RTCWriteByte(0x04, (( date_ten << 4 ) + date_one ));
}
void RTCSetWeekday( uint8_t weekday ){
	uint8_t	weekday_one = 0;
	uint8_t weekday_ten = 0;
	while ( weekday > 0){
		weekday_one ++;
		if( weekday_one == 10 ){
			weekday_one = 0;
			weekday_ten ++;
		}
		weekday --;
	}
	RTCWriteByte(0x03, (( weekday_ten << 4 ) + weekday_one ));
}
void  RTCSetHour( uint8_t hour ){
	uint8_t	hour_one = 0;
	uint8_t hour_ten = 0;
	while ( hour > 0){
		hour_one ++;
		if( hour_one == 10 ){
			hour_one = 0;
			hour_ten ++;
		}
		hour --;
	}
	RTCWriteByte(0x02, (( hour_ten << 4 ) + hour_one ));
}
void  RTCSetMinute( uint8_t minute ){
	uint8_t	minute_one = 0;
	uint8_t minute_ten = 0;
	while ( minute > 0){
		minute_one ++;
		if( minute_one == 10 ){
			minute_one = 0;
			minute_ten ++;
		}
		minute --;
	}
	RTCWriteByte(0x01, (( minute_ten << 4 ) + minute_one ));
}
void RTCSetSecond	( uint8_t second ){
	uint8_t	second_one = 0;
	uint8_t second_ten = 0;
	while ( second > 0){
		second_one ++;
		if( second_one == 10 ){
			second_one = 0;
			second_ten ++;
		}
		second --;
	}
	RTCWriteByte(0x00, (( second_ten << 4 ) + second_one ) + 0x80 );
}