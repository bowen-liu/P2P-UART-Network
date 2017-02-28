//source: https://github.com/dhylands/projects/blob/master/common/Crc8.c

#include "crc8.h"

//Calculate the  crc8 for a single byte
//Let inCRC be 0 if it's the first piece
unsigned char crc8 ( unsigned char inCrc, unsigned char inData )
{
	unsigned char   i;
    unsigned char   data;

    data = inCrc ^ inData;

	for ( i = 0; i < 8; i++ )
    {
        if (( data & 0x80 ) != 0 )
        {
            data <<= 1;
            data ^= 0x07;
        }
        else
        {
            data <<= 1;
        }
	}

	return data;
}


//Calculate the accumulated crc8 for a block of bytes
//Let inCRC be 0 if it's the first piece
uint8_t crc8_block( uint8_t crc, uint8_t *data, unsigned int len )
{
    while ( len > 0 )
    {
        crc = crc8( crc, *data++ );
        len--;
    }

    return crc;

}