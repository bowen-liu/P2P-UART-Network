#include "uart_stdout.h"

FILE serial_stdout;

int serial_putchar(char c, FILE* f)
{
	if (c == '\n') serial_putchar('\r', f);
	return Serial.write(c) == 1 ? 0 : 1;
}


void stdout_uart_init()
{
	Serial.begin(BITRATE);
	fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &serial_stdout;
}