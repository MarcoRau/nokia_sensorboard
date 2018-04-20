
#include "twi.h"

void InitRTC( void );
uint8_t RTCWriteByte ( uint8_t, uint8_t );
uint8_t RTCReadByte ( uint8_t );

// Date
void RTCSetYear		( uint8_t );
void RTCSetMonth	( uint8_t );
void RTCSetDate		( uint8_t );
void RTCSetWeekday	( uint8_t );

// Time
void RTCSetHour		( uint8_t );
void RTCSetMinute	( uint8_t );
void RTCSetSecond	( uint8_t );