#ifndef _UARTNET_CRC8H_
#define _UARTNET_CRC8H_

#include <stdio.h>
#include <stdint.h>

#define INITIAL_CRC 0x0

unsigned char crc8 ( unsigned char inCrc, unsigned char inData );
uint8_t crc8_block( unsigned char crc, unsigned char *data, unsigned int len );

#endif