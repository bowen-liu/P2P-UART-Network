#include "node.h"

LINK link;
uint8_t my_ID = 1;

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
  printf("Send queue size: %d\n", SEND_QUEUE_SIZE);

  //Initializing link layer data for serial1
  Serial1.begin(115200);
  link = link_init(&Serial1);
}


/******************************/
//Switch
/******************************/

void proc_raw_frames(RAW_FRAME raw, LINK *link)
{
  uint16_t preamble = *((uint16_t*)&raw.buf[0]);
  uint8_t dest = ((*((uint8_t*) &raw.buf[2])) >> 4);

  //Is this packet intended for the switch itself?
  if(dest == 0)
  {
    printf("Link Packet!\n");
    
    //TODO: Handle whatever link-layer logic

    free(raw.buf);
    return;
  }

  //Handle broadcast packets
  if(dest == MAX_ADDRESS)
  {
    printf("Broadcasting Packet!\n");
    
    //TODO: Handle whatever link-layer logic

    free(raw.buf);
    return;
  }

  parse_raw_frame(raw);
  free(raw.buf);

  //Forward this singular packet otherwise
  //TODO: ARP table
  //add_to_send_queue(raw, link);

}


void net_task()
{
  int i;
  size_t bytes;
  RAW_FRAME rawframe;
  
  printf("Switch is starting...\n");

  /*
  create_send_frame(0xc, 0x6, 0x13, "ABCDEFGHIJKLMNOPQRS", &link);
  create_send_frame(0xE, 0xD, 0x3c, "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE", &link);
  create_send_frame(0xA, 0xC, 0x26, "xyxyxyxyxyxyxyxyxyxxyxyxyxyxyxyxyxyxyx", &link);
  */
  
  

//Testing uspacket


  USPACKET us1 = create_uspacket(0xC, 0x6, 0xEAEA, 0x13, 0, 0x13, "ABCDEFGHIJKLMNOPQRS");

  USPACKET us2 = create_uspacket(0xE, 0xD, 0x1E9A, 0x3c, 0, 0x3c, "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE");

  USPACKET us3 = create_uspacket(0xa, 0xc, 0x69AF, 0x26, 0, 0x26, "xyxyxyxyxyxyxyxyxyxxyxyxyxyxyxyxyxyxyx");

  printf("\n**************\n");
  print_uspacket(us1);
  print_uspacket(us2);
  print_uspacket(us3);
  printf("\n**************\n");
  
  send_uspacket(us1, &link);
  send_uspacket(us2, &link);
  send_uspacket(us3, &link);



  printf("All sent! \n");

  
  while (1)
  {
    //See if any new bytes are available for reading
    bytes = check_link_rw(&link);

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
  }
}

void loop()
{
  net_task();
}




