#ifndef _UARTNET_UART_STDOUTH_
#define _UARTNET_UART_STDOUTH_

#include <stdlib.h>
#include <stdio.h>
#include "Arduino.h"

#define BITRATE 115200

int serial_putchar(char c, FILE* f);
void stdout_uart_init();

#endif

