#include "node.h"

TRANSPORT tr;
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

  //Initializing transport layer data for serial1
  Serial1.begin(115200);
  link = link_init(&Serial1);
  //tr = transport_initialize();
  //transport_register_link(&tr, Serial1);
  
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

  printf("\n****************************\n");
  //FRAME frame = raw_to_frame(raw);
  //print_frame(frame);
  parse_raw_and_store(raw, link);
  //free(frame.payload);
  printf("\n****************************\n");

  

  //Forward this singular packet otherwise
  //TODO: ARP table
  //add_to_send_queue(raw, link);

}


void net_task()
{
  size_t bytes;
  RAW_FRAME rawframe;
  
  printf("Switch is starting...\n");

  /*
  create_send_frame(0xc, 0x6, 0x13, "ABCDEFGHIJKLMNOPQRS", &link);
  create_send_frame(0xE, 0xD, 0x3c, "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE", &link);
  create_send_frame(0xA, 0xC, 0x26, "xyxyxyxyxyxyxyxyxyxxyxyxyxyxyxyxyxyxyx", &link);
  */
  
  

//Testing uspacket

/*
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

*/

//Testing RSPackets


RSPACKET syn1 = create_rspacket_syn(0xE, 0xD, 0x3c);
send_rspacket(syn1, &link);
RSPACKET ack1 = create_rspacket_ack(0xE, 0xD, 0x3c, 0x1E9A, 0);
send_rspacket(ack1, &link);
RSPACKET dat1 = create_rspacket_data(0xE, 0xD, 0x3c, 0x1E9A, 0, "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE");
send_rspacket(dat1, &link);


create_send_frame(0xc, 0x6, 0x13, "ABCDEFGHIJKLMNOPQRS", &link);
create_send_frame(0xA, 0xC, 0x26, "xyxyxyxyxyxyxyxyxyxxyxyxyxyxyxyxyxyxyx", &link);
create_send_frame(0xE, 0xD, 0x3c, "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE", &link);





  printf("All sent! \n");

  int i = 0, j=0;

  
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

    
    if(i >= 5)
    {
      //print buf
      for(j = 0; j<RECV_QUEUE_SIZE; j++)
      {
        if(link.recv_queue[j].size > 0)
        {
          printf("recv_queue pos %d\n",j);
          print_frame(link.recv_queue[j]);
        }
      }

      printf("\n**************************************\n");
      int curpending = link.rqueue_pending;
      printf("pending: %d\n", curpending);
      
      for(j = 0; j<curpending; j++)
      {
         printf("\nloop %d\n", j);
         
         FRAME retframe = pop_recv_queue(&link);
          print_frame(retframe);
         free(retframe.payload);
      }
      
      i = 0;
    }
    else i++;

      
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




