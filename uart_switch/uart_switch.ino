#include "switch.h"

LINK links[TOTAL_LINKS];

/******************************/
//Arduino Setup
/******************************/

FILE serial_stdout;

int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
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
   links[0] = link_init(&Serial1);
}


/******************************/
//Switch
/******************************/

void probe_all()
{
  
  //create_frame(0, MAX_ADDRESS, sizeof(PROBE_MSG), PROBE_MSG);
}


void proc_raw_frames(RAW_FRAME raw, LINK *link)
{
  uint16_t preamble = *((uint16_t*)&raw.buf[0]);
  uint8_t dest = ((*((uint8_t*) &raw.buf[2])) >> 4);
  int i;

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
  FRAME frame = raw_to_frame(raw);
  print_frame(frame);
  free(frame.payload);
  printf("\n****************************\n");

  //Forward this singular packet otherwise
  //TODO: ARP table
  add_to_send_queue(raw, link);

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
  
  while(1)
  {
     //Process one serial port at a time
     for(i=0; i<TOTAL_LINKS; i++)
     {
       //See if any new bytes are available for reading
       bytes = check_new_bytes(&links[i]);
       
       //Check if buffer contains one or more complete packets
       if(bytes > 0 || links[i].rbuf_valid)
       {
         rawframe = extract_frame_from_rbuf(&links[i]);
         while(rawframe.size > 0)
         {
            proc_raw_frames(rawframe, &links[i]);
  
            //Check if the rbuf contains more complete packets
            proc_buf(NULL, 0, &links[i]);
            rawframe = extract_frame_from_rbuf(&links[i]);
         }
       }
       
       //Transmit a packet in the sending queue, if any
       transmit_next(&links[i]);
     }
     
  }
}




void loop() 
{
 switch_task();
}




