#include "switch.h"


LINK links[TOTAL_LINKS];

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

  //Initializing link layer data for serial1
  Serial1.begin(115200);
  links[0] = link_init(&Serial1, 0, GATEWAY);
  Serial2.begin(115200);
  links[1] = link_init(&Serial2, 0, GATEWAY);
  Serial3.begin(115200);
  links[2] = link_init(&Serial3, 0, GATEWAY);

  //Send out a HELLO message out onto the link
  send_probe_msg(0, &links[0]);
  send_probe_msg(0, &links[1]);
  send_probe_msg(0, &links[2]);
}


/******************************/
//Switch
/******************************/

uint8_t broadcast(FRAME frame)
{
  uint8_t i, j;
  uint8_t sent_count = 0;
  uchar* pl_orig = frame.payload;

  //Loop through every link in the switch
  for (i = 0; i < TOTAL_LINKS; i++) 
  {
    //Loop through every routing table entry for each link. Skipping the 0th entry        
    for (j = 1; j < RTABLE_LENGTH; j++) 
    {
      //Forward the frame if routing entry is valid, and it's not the sender    
      if (links[i].rtable[j].id == j && links[i].rtable[j].id != frame.src) 
      {
        printf("Broadcasting to %d\n", j);

        //Make a new copy of the payload for each frame, since the transmitter will free them after transmission
        frame.payload = malloc(frame.size);
        memcpy(frame.payload, pl_orig, frame.size);

        //Change the destination to the current endpoint and transmit
        frame.dst = j;
        send_frame(frame, &links[i]);
        ++sent_count;
      }
    }
  }

  //Assuming raw.buf is freed by the caller

  return sent_count;
}


void proc_raw_frames(RAW_FRAME raw, LINK *link)
{
  uint16_t preamble = *((uint16_t*)&raw.buf[0]);
  uint8_t dest = ((*((uint8_t*) &raw.buf[2])) >> 4);
  int i;

  FRAME frame;

  //Is this packet intended for the switch itself?
  if (dest == 0)
  {
    //Parse the raw frame
    frame = raw_to_frame(raw);
    free(raw.buf);

    parse_routing_frame(frame, link);
    free(frame.payload);
    return;
  }

  //Handle broadcast packets
  if (dest == MAX_ADDRESS)
  {
    //Parse the raw frame
    frame = raw_to_frame(raw);
    free(raw.buf);
    
    printf("Broadcast!\n");
    printf("Broadcasted frame to %d nodes\n", broadcast(frame));
    free(frame.payload);
    return;
  }

  //Find which port is the dst reachable
  for (i = 0; i < TOTAL_LINKS; i++)
  {
    if (links[i].rtable[dest].id == dest)
      break;
    else if (i == TOTAL_LINKS - 1)
    {
      printf("Could not locate node %d in any routing tables! Dropping...\n", dest);
      free(raw.buf);
      return;
    }
  }

  //Forward the frame
  add_to_send_queue(raw, &links[i]);

}


uint8_t read_serial_raw(LINK *link)
{
  uint8_t frames_received = 0;
  size_t bytes;
  RAW_FRAME rawframe;

  //See if any new bytes are available for reading
  bytes = check_new_bytes(link);

  //Check if buffer contains one or more complete packets
  if (!(bytes > 0 || link->rbuf_valid))
    return 0;

  //Extract frames from the raw receive buffer
  rawframe = extract_frame_from_rbuf(link);
  while (rawframe.size > 0)
  {
    //Parse the frame and store it in the recvd buffer
    proc_raw_frames(rawframe, link);

    //Check if the rbuf contains more complete packets
    proc_buf(NULL, 0, link);
    rawframe = extract_frame_from_rbuf(link);
    frames_received++;
  }

  return frames_received;
}


/******************************/
//main
/******************************/


void switch_task()
{
  int i;
  size_t bytes;
  RAW_FRAME rawframe;

  printf("Switch is starting...\n");

  while (1)
  {
    //Process one serial port at a time
    for (i = 0; i < TOTAL_LINKS; i++)
    {
      //Check if current serial port has any new frames ready for reading
      read_serial_raw(&links[i]);

      //Transmit a packet in the sending queue, if any
      transmit_next(&links[i]);
    }
  }
}




void loop()
{
  switch_task();
}




