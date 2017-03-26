#include "node.h"

//TRANSPORT tr;
LINK link;
uint8_t my_id = 1;

/******************************/
//Arduino Setup
/******************************/

FILE serial_stdout;
int serial_putchar(char c, FILE* f)
{
  if (c == '\n') serial_putchar('\r', f);
  return Serial.write(c) == 1 ? 0 : 1;
}


void setup()
{
  //Setting up serial1 and stdout redirection
  Serial.begin(115200);
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;

  //Initializing transport layer data for serial1
  Serial1.begin(115200);
  link = link_init(&Serial1, my_id, ENDPOINT);
  //tr = transport_initialize();

  //Send out a HELLO message out onto the link
  send_hello(my_id, 0, &link);
}


/******************************/
//Node functions
/******************************/


void proc_frame(FRAME frame, LINK *link)
{
  //Is this packet from the switch itself?
  if (frame.src == 0)
  {
    parse_routing_frame(frame, link);
    free(frame.payload);
    return;
  }

  //Handle broadcast packets. End nodes will treat broadcast frames as normal frames
  if (frame.dst == MAX_ADDRESS)
  {
    printf("Broadcasting Packet!\n");

    //TODO: Handle whatever link-layer logic
    print_frame(frame);
    printf("\n");

    free(frame.payload);
    return;
  }

  //Print out any other frames received
  print_frame(frame);

  //Send the received frame to the transport layer for further processing
  //parse_recvd_frame(frame);

}



void net_task()
{
  uint8_t done = 0;
  printf("Node is starting...\n");



  while (1)
  {
    //Attempt to read serial. Process any pending frames if received anything new
    if (read_serial(&link) > 0)
    {
      while (link.rqueue_pending > 0)
        proc_frame(pop_recv_queue(&link), &link);
    }

    //Transmit a packet in the sending queue, if any
    transmit_next(&link);
    delay(100);


    //Testing network join
    if (link.end_link_type != UNKNOWN && done == 0)
    {
      done = 1;

      send_join_msg(my_id, &link);
      send_join_msg(3, &link);
      send_join_msg(14, &link);

      delay(3000);
      continue;
    }

    //Send out test packets
    if (done == 1)
    {
      done = 2;

      //Test routing
      printf("\n\n*TESTING BASIC ROUTING*\n\n");
      create_send_frame(my_id, 3, 8, "FROM1TO3", &link);      //ok
      create_send_frame(my_id, 4, 8, "FROM1TO4", &link);      //not ok
      create_send_frame(my_id, 14, 9, "FROM1TO14", &link);    //ok
      create_send_frame(my_id, 3, 8, "hellolol", &link);      //ok
      create_send_frame(my_id, 5, 5, "ifail", &link);         //not ok



      //Broadcast test
      printf("\n\n*TESTING BROADCAST*\n\n");
      create_send_frame(my_id, MAX_ADDRESS, 10, "HELLOWORLD", &link);
    }

  }
}

void loop()
{
  net_task();
}




