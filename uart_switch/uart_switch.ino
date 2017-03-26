#include "switch.h"

LINK links[TOTAL_LINKS];

/******************************/
//Arduino Setup
/******************************/

void timer1_isr()
{
  int i;
  for (i = 0; i < TOTAL_LINKS; i++)
    check_alive(&links[i]);
}


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

  //Setup Timer
  //Timer1.initialize(TICK_MS);
  //Timer1.attachInterrupt(timer1_isr);

  //Initializing link layer data for serial1
  Serial1.begin(115200);
  links[0] = link_init(&Serial1, 0, GATEWAY);
  Serial2.begin(115200);
  links[1] = link_init(&Serial2, 0, GATEWAY);
  Serial3.begin(115200);
  links[2] = link_init(&Serial3, 0, GATEWAY);

  //Send out a HELLO message out onto the link
  send_hello(0, 0, &links[0]);
  send_hello(0, 0, &links[1]);
  send_hello(0, 0, &links[2]);
}


/******************************/
//Switch
/******************************/

void check_alive(LINK *link)
{
  int i, j;

  //Loop through every routing table entry for each link. Skipping the 0th entry
  for (j = 1; j < RTABLE_LENGTH; j++)
  {
    //Increment the tick count on every live end node
    if (link->rtable[j].hops == 1)
    {
      //Increment the tick count for the current live node
      ++link->rtable[j].ticks;

      //Mark the node dead if tick threshold has been exceeded
      if (link->rtable[j].ticks >= PING_TICKS_THRESHOLD)
      {
        link->rtable[j].hops = 0;
        printf("ALERT: Node %d is declared dead!\n", j);

        //Broadcast a LEAVE message if switch
      }

      //Ping the node if missed ticks has exceeded to PING_TICKS
      else if (link->rtable[j].ticks >= PING_TICKS)
      {
        printf("Checking if %d is alive\n", j);
        send_hello(0, j, link);
      }
    }
  }
}

void reset_tick(uint8_t id)
{
  int i;

  for (i = 0; i < TOTAL_LINKS; i++) {
    if (links[i].rtable[id].hops > 0) {
      links[i].rtable[id].ticks = 0;
      break;
    }
  }

  if (i == TOTAL_LINKS && links[i].rtable[id].hops == 0)
    printf("ERROR: Could not find id %d\n", id);
}

uint8_t broadcast(FRAME frame)
{
  uint8_t i, j;
  uint8_t sent_count = 0;
  uchar* pl_orig = frame.payload;

  //Loop through every link in the switch
  for (i = 0; i < TOTAL_LINKS; i++)
  {
    //Do not forward if the other end of the link is uninitialized, or the other end of the link is only the sender
    if (links[i].end_link_type == UNKNOWN ||
        (links[i].rtable[frame.src].hops == 1 && links[i].rtable_entries == 1))
      continue;

    //Make a new copy of the payload for each frame, since the transmitter will free them after transmission
    frame.payload = malloc(frame.size);
    memcpy(frame.payload, pl_orig, frame.size);

    //Change the destination to the current endpoint and transmit
    send_frame(frame, &links[i]);
    ++sent_count;
  }

  //Assuming raw.buf is freed by the caller

  return sent_count;
}


void proc_raw_frames(RAW_FRAME raw, LINK *link)
{
  uint16_t preamble = *((uint16_t*)&raw.buf[0]);
  uint8_t src = (*((uint8_t*) &raw.buf[2])) & 0x0F;
  uint8_t dest = ((*((uint8_t*) &raw.buf[2])) >> 4);
  int i;

  FRAME frame;

  //Reset the missed tick count for the sender
  reset_tick(src);

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
    if (links[i].rtable[dest].hops > 0)
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




