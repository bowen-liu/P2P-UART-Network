#include "node.h"

TRANSPORT tr;
LINK link;
uint8_t my_id = 1;

/******************************/
//Arduino Setup
/******************************/

FILE serial_stdout;

int serial_putchar(char c, FILE* f) {
  if (c == '\n') serial_putchar('\r', f);
  return Serial.write(c) == 1 ? 0 : 1;
}

void setup()
{
  //Setting up serial1 and stdout redirection
  Serial.begin(115200);
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;

  printf("Serial Buffer: %d\n", SERIAL_RX_BUFFER_SIZE );

  //Initializing transport layer data for serial1
  Serial1.begin(115200);
  link = link_init(&Serial1, ENDPOINT);
  tr = transport_initialize();


  //Send out a HELLO message out onto the link
  send_probe_msg(my_id, &link);
}


/******************************/
//Node functions
/******************************/



void proc_raw_frames(RAW_FRAME raw, LINK *link)
{
  FRAME frame;

  //Parse the raw frame 
  frame = raw_to_frame(raw);
  free(raw.buf);

  //Is this packet from the switch itself?
  if(frame.src == 0)
  {
    parse_routing_frame(frame, link);
    free(frame.payload);
    return;
  }

  //Handle broadcast packets
  if(frame.dst == MAX_ADDRESS)
  {
    printf("Broadcasting Packet!\n");
    
    //TODO: Handle whatever link-layer logic

    free(raw.buf);
    return;
  }



  //Send the received frame to the transport layer for further processing
  parse_recvd_frame(frame);


}


void net_task()
{
  uint8_t done = 0;
  size_t bytes;
  RAW_FRAME rawframe;
  
  printf("Node is starting...\n");

  
  
  while (1)
  {

    //See if any new bytes are available for reading
    bytes = check_new_bytes(&link);

    //Check if buffer contains one or more complete packets
    if (bytes > 0 || link.rbuf_valid)
    {
      rawframe = extract_frame_from_rbuf(&link);
      while (rawframe.size > 0)
      {
        proc_raw_frames(rawframe, &link);

        //Check if the rbuf contains more complete packets
        proc_buf(NULL, 0, &link);
        rawframe = extract_frame_from_rbuf(&link);
      }
    }

    //Transmit a packet in the sending queue, if any
    transmit_next(&link);
    delay(100);


    //Send out test packets
    if(link.end_link_type != UNKNOWN && !done)
    {
      done = 1;
      
      send_join_msg(my_id, &link);
      send_join_msg(3, &link);
      send_join_msg(14, &link);
    }

  }
}

void loop()
{
  net_task();
}




