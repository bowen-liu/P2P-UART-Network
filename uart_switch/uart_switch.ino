#include "switch.h"
#include <uart_stdout.h>


void setup()
{
  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  switch_init();
}


void loop()
{
  switch_task(1);
}




