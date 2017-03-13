/*This file must be declared as a .cpp file since it uses the HardwareSerial Object from Arduino's library*/
#include "link.h"

static uint8_t links_registered = 0;

LINK link_init(HardwareSerial *port)
{
  LINK link;

  link.port = port;
  link.id = links_registered++;
  
  link.rbuf_writeidx = 0;
  link.rbuf_valid = 0;
  link.rbuf_expectedsize = 0;
  link.squeue_pending = 0;
  link.squeue_lastsent = 0;

  memset(link.recvbuf, 0, RECV_BUFFER_SIZE);
  memset(link.send_queue, 0, SEND_QUEUE_SIZE * sizeof(RAW_FRAME));

  return link;
}



/***************************
RECEIVING FRAMES
***************************/



void proc_buf(uchar *rawbuf, size_t chunk_size, LINK *link)
{
  int i;
  uint16_t preamble;
  int valid_bytes;

  //append the new chunk to the appropriate offset of the buffer
  //If function is called without a new chunk, simply check if the current buffer contains pieces of a subsequent packet
  if (rawbuf != NULL && chunk_size > 0)
  {
    memcpy(&link->recvbuf[link->rbuf_writeidx], rawbuf, chunk_size);
    link->rbuf_writeidx += chunk_size;
  }

  //If the buffer is current marked invalid, try and find a preamble to match a new packet
  if (!link->rbuf_valid)
  {
    for (i = 0; i < link->rbuf_writeidx; i++ )
    {
      preamble = *((uint16_t*) &link->recvbuf[i]);
      
      if (preamble == FRAME_PREAMBLE)
      {
        printf("Found a preamble: %X\n", preamble);      
        memcpy(&link->recvbuf[0], &link->recvbuf[i], (RECV_BUFFER_SIZE - i));
        link->rbuf_writeidx -= i;
        link->rbuf_valid = 1;
        break;
      }
    }

    //Flush the buffer if nothing has been found, and the buffer is >50% full
    if (link->rbuf_writeidx >= FLUSH_THRESHOLD && !link->rbuf_valid)
    {
      printf("Flushing %d bytes of unknown raw chunk \n", link->rbuf_writeidx);
      link->rbuf_writeidx = 0;
    }
  }
}




size_t check_complete_frame(LINK *link)
{
  uint16_t preamble = *((uint16_t*)&link->recvbuf[0]);

  if (preamble != FRAME_PREAMBLE)
    return 0;

  //Set the expected payload size if full header has received
  if (link->rbuf_writeidx >= FRAME_HEADER_SIZE)
    link->rbuf_expectedsize = FRAME_HEADER_SIZE + *((uint8_t*)&link->recvbuf[3]) + 2;  //add 2 bytes for "STX" and "ETX" for payload

  //Check total length of raw packet received
  if (link->rbuf_writeidx >= link->rbuf_expectedsize )
    return link->rbuf_expectedsize;
  else
    return 0;

}


/*
  size_t check_link_rw(LINK *link)
  {
  size_t bytes;
  int partial_size, pos;
  uchar tempbuf[RECV_BUFFER_SIZE];

   bytes = link->port->available();
     if(bytes <= 0)
      return 0;

     bytes = link->port->readBytes(tempbuf, bytes);

     //Make sure we're not overflowing the buffer
     while(bytes > 0)
     {
       if(link->rbuf_writeidx + bytes < RECV_BUFFER_SIZE)
       {
          proc_buf(tempbuf, bytes, link);
          pos = 0;
          return bytes;
       }
       else
       {
          //TODO: This part needs more througho testing, and doesn't seem to be working properly

          printf("***too big! cur_idx: %d bytes_pending: %d\n", link->rbuf_writeidx, bytes);

          partial_size = bytes - (RECV_BUFFER_SIZE - link->rbuf_writeidx);
          bytes -= partial_size;

          printf("processing subchunk: %d\n", partial_size);
          proc_buf(&tempbuf[pos], partial_size, link);

          pos += partial_size;
          break;
       }
     }

     return bytes;

  }
*/

size_t check_link_rw(LINK *link)
{
  int bytes;
  int partial_size, pos;
  uchar tempbuf[RECV_BUFFER_SIZE];

  bytes = link->port->available();
  if (bytes <= 0)
    return 0;

  bytes = link->port->readBytes(tempbuf, bytes);
  
  //Make sure we're not overflowing the buffer
  if (link->rbuf_writeidx + bytes < RECV_BUFFER_SIZE)
  {
    proc_buf(tempbuf, bytes, link);
    pos = 0;
    return bytes;
  }
  else
  {
    //TODO: This part needs more througho testing, and doesn't seem to be working properly
    printf("***too big! cur_idx: %d bytes_pending: %d\n", link->rbuf_writeidx, bytes);
    return 0;
  }
}


RAW_FRAME extract_frame_from_rbuf(LINK *link)
{
  RAW_FRAME raw_frame;
  raw_frame.size = check_complete_frame(link);

  //If the buffer doesn't have any fully received packets, return size 0
  if (raw_frame.size <= 0)
    return raw_frame;

  printf("Complete packet received! %u bytes!\n", raw_frame.size);
  print_bytes(&link->recvbuf[0], raw_frame.size);

  //Allocate a new buffer for the raw packet for returning
  raw_frame.buf = malloc(raw_frame.size);
  memcpy(raw_frame.buf, link->recvbuf, raw_frame.size);

  link->rbuf_valid = 0;
  link->rbuf_expectedsize = 0;

  //Move write pointer to the end of the packet
  link->rbuf_writeidx -= raw_frame.size;
  memcpy(&link->recvbuf[0], &link->recvbuf[raw_frame.size], (RECV_BUFFER_SIZE - raw_frame.size));

  return raw_frame;
}


//Turn a raw frame buffer into a FRAME struct. If intended for link layer, handle it immediately
FRAME parse_raw_frame(RAW_FRAME raw)
{
  FRAME frame;  
  uint16_t preamble = *((uint16_t*)&raw.buf[0]);
  int i;
  
  //print_bytes(buf, bytes);

  //Parse the header
  frame = buf_to_frame(raw.buf);

  //Check if payload is complete
  if (raw.buf[FRAME_HEADER_SIZE] != STX)
    printf("STX not found, packet header may be corrupt!\n");
  if (raw.buf[FRAME_HEADER_SIZE + frame.size + 1] != ETX)
    printf("ETX not found, packet may be corrupt or payload was truncated!\n");


  printf("\n****************************\n");
  print_frame(frame);
  printf("payload: ");
  print_bytes(frame.payload, frame.size);
  printf("\n****************************\n");

  //Remember to free its payload when done!

  return frame;
}




/***************************
SENDING FRAMES
***************************/

uint8_t add_to_send_queue(RAW_FRAME raw, LINK *link)
{
  int i, j;

  if (link->squeue_pending == SEND_QUEUE_SIZE )
  {
    printf("Send Queue is full! Dropping request...\n");
    free(raw.buf);
    return 0;
  }

  //Find a free slot in the sending queue, starting from the last sending index
  for (i = link->squeue_lastsent + 1, j = 0; j < SEND_QUEUE_SIZE; j++)
  {
    //Wrap index i around to the beginning if needed
    if (i >= SEND_QUEUE_SIZE - 1) i = 0;
    else if (i < SEND_QUEUE_SIZE - 1) i++;

    if (link->send_queue[i].size == 0) break;
  }

  link->send_queue[i] = raw;
  link->squeue_pending++;

  return 1;
}


uint8_t transmit_next(LINK *link)
{
  uint8_t i, j;

  //Check if we have anything to transmit
  if (link->squeue_pending == 0)
    return 0;

  //Find the next packet to transmit, starting from the last sending index
  for (i = link->squeue_lastsent, j = 0; j < SEND_QUEUE_SIZE; j++)
  {
    //Wrap index i around to the beginning if needed
    if (i >= SEND_QUEUE_SIZE - 1) i = 0;
    else if (i < SEND_QUEUE_SIZE - 1) i++;

    if (link->send_queue[i].size > 0) break;
  }

  //printf("PRETENDING to transmit: %d ", i);
  //print_bytes(link->send_queue[i].buf, link->send_queue[i].size);
  
  //Transmit the packet out onto the link
  link->port->write(link->send_queue[i].buf, link->send_queue[i].size);

  
  //cleanup & mark this slot as free
  link->squeue_lastsent = i;
  link->squeue_pending--;
  link->send_queue[i].size = 0;
  free(link->send_queue[i].buf);

  return i;
}

uint8_t send_frame(FRAME frame, LINK *link)
{
	return add_to_send_queue(frame_to_buf(frame), link);
}


uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link)
{
	return add_to_send_queue(frame_to_buf(create_frame(src, dst, size, payload)), link);
}









