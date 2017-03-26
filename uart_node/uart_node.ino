#include "node.h"
#include <uart_stdout.h>

void setup()
{
  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  //Setup the network
  node_init();
}


void loop()
{
  net_task(1);
}




