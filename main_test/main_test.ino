#include "switch.h"

static uchar recvbuf[RECV_BUFFERS][RECV_BUFFER_SIZE];

static uint16_t rbuf_writeidx[RECV_BUFFERS];      //TODO: Make this into a circular buffer
static uint8_t rbuf_valid[RECV_BUFFERS];
static uint16_t rbuf_expectedsize[RECV_BUFFERS];


/******************************/
//Arduino Setup
/******************************/

FILE serial_stdout;

int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}


void initialize()
{
  int i;
  
  //set all write indexes to zero
 memset(rbuf_writeidx, 0, RECV_BUFFERS*sizeof(uint16_t));
 memset(rbuf_valid, 0, RECV_BUFFERS*sizeof(uint8_t));
 memset(rbuf_expectedsize, 0, RECV_BUFFERS*sizeof(uint16_t));

 
}


void setup()
{
   //Setting up serial1 and stdout redirection
   Serial.begin(115200);
   Serial1.begin(115200);
   //Serial1.begin(9600);
   fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
   stdout = &serial_stdout;

   printf("Serial Buffer: %d\n", SERIAL_RX_BUFFER_SIZE );

   initialize();
}


/******************************/
//Switch
/******************************/



void probe_all()
{
  
  //create_umpacket(0, MAX_ADDRESS, sizeof(PROBE_MSG), PROBE_MSG);
}


void proc_buf(uchar *rawbuf, size_t chunk_size, uint8_t idx)
{
  int i;
  uint16_t preamble;
  int valid_bytes;

  //append the new chunk to the appropriate offset of the buffer
  //If function is called without a new chunk, simply check if the current buffer contains pieces of a subsequent packet

  if(rawbuf != NULL && chunk_size > 0)
  {
      memcpy(&recvbuf[idx][rbuf_writeidx[idx]], rawbuf, chunk_size);
      rbuf_writeidx[idx] += chunk_size;
  }


  //If the buffer is current marked invalid, try and find a preamble to match a new packet
  if(!rbuf_valid[idx])
  {
    for(i=0; i<rbuf_writeidx[idx]; i++ )
    {
      preamble = *((uint16_t*) &recvbuf[idx][i]);
      //printf("preamble: %u \n", preamble);
      
      if(preamble == UMPACKET_PREAMBLE ||  preamble == USPACKET_PREAMBLE || preamble == RSPACKET_SYN_PREAMBLE || preamble == RSPACKET_ACK_PREAMBLE || preamble == RSPACKET_DATA_PREAMBLE)
      {
        printf("Found a preamble: %X\n", preamble);
        rbuf_valid[idx] = 1;

        //move the preamble and everything after it to the front of the buffer
        //valid_bytes = rbuf_writeidx[idx] - i;
        //memcpy(&recvbuf[idx][0], &recvbuf[idx][i], valid_bytes);
        //rbuf_writeidx[idx] = valid_bytes;
        
        memcpy(&recvbuf[idx][0], &recvbuf[idx][i], (RECV_BUFFER_SIZE - i));
        rbuf_writeidx[idx] -= i;

        break;
      }

      //printf("write idx: %d\ncurrent: ", rbuf_writeidx[idx]); print_bytes(&recvbuf[idx][0], RECV_BUFFER_SIZE);
    }

       //Flush the buffer if nothing has been found, and the buffer is >50% full
      if(rbuf_writeidx[idx] >= FLUSH_THRESHOLD && !rbuf_valid[idx])
      {
        printf("Flushing %d bytes of unknown raw chunk \n", rbuf_writeidx[idx]);
        rbuf_writeidx[idx] = 0;
      }
    
  }
}


int check_packet_complete(uint8_t idx)
{
   uint16_t preamble = *((uint16_t*)&recvbuf[idx][0]);
  
  //Extract the SIZE field, depending on the preamble
  switch(preamble)
  {
    
    case UMPACKET_PREAMBLE:
    
     //Set the expected payload size if full header has received
     if(rbuf_writeidx[idx] >= UMPACKET_HEADER_SIZE)
     {
      rbuf_expectedsize[idx] = *((uint8_t*)&recvbuf[idx][3]) + UMPACKET_HEADER_SIZE;
      //printf("packet size: %d\n", rbuf_expectedsize[idx]);
     }
    
      //Check total length of raw packet received
      if(rbuf_writeidx[idx] >= rbuf_expectedsize[idx] )
        return  rbuf_expectedsize[idx];
      else
        return 0;

        

      
    case USPACKET_PREAMBLE:
      if(rbuf_writeidx[idx] >= USPACKET_HEADER_SIZE - 1 )
        rbuf_expectedsize[idx] = *((uint8_t*)&recvbuf[idx][9]);
        
      if(rbuf_writeidx[idx] >= rbuf_expectedsize[idx] + USPACKET_HEADER_SIZE)
        return rbuf_writeidx[idx];
      else
        return 0;

      
    case RSPACKET_SYN_PREAMBLE:
    case RSPACKET_ACK_PREAMBLE:
      rbuf_expectedsize[idx] =  0;
      if(rbuf_writeidx[idx] >= RSPACKET_HEADER_SIZE)
        return rbuf_writeidx[idx];
      else
        return 0;
        
      
    case RSPACKET_DATA_PREAMBLE:
      if(rbuf_writeidx[idx] >= RSPACKET_HEADER_SIZE)
        rbuf_expectedsize[idx] = *((uint8_t*)&recvbuf[idx][5]);
        
      if(rbuf_writeidx[idx] >= rbuf_expectedsize[idx] + RSPACKET_HEADER_SIZE)
        return rbuf_writeidx[idx];
      else
        return 0;
    
    default:
      //printf("check_packet_complete: invalid preamble at the start of rbuf\n");
      return -1;
  }
}


/******************************/
//main
/******************************/


void switch_task()
{
  int bytes;
  int partial_size, pos;
  uchar tempbuf[RECV_BUFFER_SIZE];

  printf("Switch is starting...\n");
  
  while(1)
  {
      //Process Serial1
      bytes = Serial1.available();
     if(bytes <= 0)
      goto HANDLE_SERIAL2;
      
     //printf("Receiving %d bytes from Serial1\n", bytes);
     bytes = Serial1.readBytes(tempbuf, bytes);

     //Make sure we're not overflowing the buffer
     while(bytes > 0)
     {
       if(rbuf_writeidx[1] + bytes < RECV_BUFFER_SIZE)
       {
          proc_buf(tempbuf, bytes, 1);
          pos = 0;
          break;
       }
       else
       {
          //TODO: This part needs more througho testing, and doesn't seem to be working properly
          
          printf("***too big! cur_idx: %d bytes_pending: %d\n", rbuf_writeidx[1], bytes);
          
          partial_size = bytes - (RECV_BUFFER_SIZE - rbuf_writeidx[1]);
          bytes -= partial_size;
          
          printf("processing subchunk: %d\n", partial_size);
          proc_buf(&tempbuf[pos], partial_size, 1);

          pos += partial_size;
          break;
       }
     }
    

     if(!rbuf_valid[1])
      goto HANDLE_SERIAL2;

     bytes = check_packet_complete(1);
     while(bytes > 0)
     {
      printf("Complete packet received! %d bytes!\n", bytes);
      print_bytes(&recvbuf[1][0], bytes);

      rbuf_valid[1] = 0;
      rbuf_expectedsize[1] = 0;

      //Move write pointer to the end of the packet
      rbuf_writeidx[1] -= bytes;
      memcpy(&recvbuf[1][0], &recvbuf[1][bytes], (RECV_BUFFER_SIZE - bytes));
      
      //Check if buffer contains further packets
      proc_buf(NULL, 0, 1);
      bytes = check_packet_complete(1);
      
      
     }



     HANDLE_SERIAL2:
     continue;
  }

  
}




void loop() 
{
 switch_task();
}




