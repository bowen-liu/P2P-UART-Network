#include "node.h"
#include <uart_stdout.h>

LINK *link;

uint8_t msg_src_expected = 2;
uint8_t msg_dst = 1;

void mframe_parser(FRAME frame)
{
  if(frame.src != msg_src_expected)
  {
    printf("Unexpected frame from %d\n", frame.src);
    print_frame(frame);
    return;
  }
  
  if(strncmp(frame.payload, "!PING", 5) == 0)
  {
    printf("Ping from %u\n", frame.src);
    create_send_frame(link->id, msg_dst, 5, "!PONG", link);
  }
  else if(strncmp(frame.payload, "!PONG", 5) == 0)
  {
    printf("Pong from %u\n", frame.src);
    create_send_frame(link->id, msg_dst, 5, "!PING", link);
  }
  else
  {
    printf("UNKNOWN from %u\n", frame.src);
    print_frame(frame);
  }
  delay(500);

}

void setup()
{
  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  //Setup the network
  link = node_init(3, mframe_parser);
  send_join_msg(link->id, link);

  //Initial
  create_send_frame(link->id, msg_dst, 5, "!PING", link);
}


void loop()
{
  net_task(0);
}




