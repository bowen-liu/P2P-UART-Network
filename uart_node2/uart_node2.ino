#include "node.h"
#include <uart_stdout.h>

LINK *link;

void mframe_parser(FRAME frame)
{
  if(strncmp(frame.payload, "!PING", 5) == 0)
  {
    printf("Ping from %u\n", frame.src);
    create_send_frame(link->id, frame.src, 5, "!PONG", link);
  }
  else if(strncmp(frame.payload, "!PONG", 5) == 0)
  {
    printf("Pong from %u\n", frame.src);
    create_send_frame(link->id, frame.src, 5, "!PING", link);
  }
  else
  {
    printf("UNKNOWN from %u\n", frame.src);
    print_frame(frame);
  }




  delay(1000);

}

void setup()
{
  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  //Setup the network
  link = node_init(2, mframe_parser);
  send_join_msg(link->id, link);

  //Initial
  create_send_frame(link->id, 1, 5, "!PONG", link);
}


void loop()
{
  net_task(0);
}




