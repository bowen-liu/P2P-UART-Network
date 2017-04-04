/*This file must be declared as a .cpp file since it uses the HardwareSerial Object from Arduino's library*/
#include "link.h"

void link_init(HardwareSerial *port, uint8_t my_id, LINK_TYPE link_type, LINK *link)
{
  link->port = port;
  link->link_type = link_type;
  link->end_link_type = UNKNOWN;
  link->id = my_id;
  
  link->rbuf_writeidx = 0;
  link->rbuf_valid = 0;
  link->rbuf_expectedsize = 0;
  link->rqueue_pending = 0;
  link->rqueue_head = 0;
  link->squeue_pending = 0;
  link->squeue_lastsent = 0;
  link->rtable_entries = 0;

  
  memset(link->recvbuf, 0, RECV_BUFFER_SIZE);
  memset(link->recv_queue, 0, RECV_QUEUE_SIZE * sizeof(FRAME));
  memset(link->send_queue, 0, SEND_QUEUE_SIZE * sizeof(RAW_FRAME));
  
  //memset(link.rtable, 0, RTABLE_LENGTH * sizeof(NODE));
  for(int i=0; i<RTABLE_LENGTH; i++ )
  {
	link->rtable[i].hops = 0;
	link->rtable[i].rtt = 0;
	link->rtable[i].ticks = 0;
	link->rtable[i].last_ping_recvd = 0;
	link->rtable[i].last_ping_sent = 0;
  }
  

}


/***************************
RECEIVING BYTES
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
      
      if (preamble == MFRAME_PREAMBLE || preamble == CFRAME_PREAMBLE)
      {
        //printf("Found a preamble: %X\n", preamble);      
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

  if (preamble != MFRAME_PREAMBLE && preamble != CFRAME_PREAMBLE)
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



size_t check_new_bytes(LINK *link)
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

  /*
  printf("Complete packet received! %u bytes!\n", raw_frame.size);
  print_bytes(&link->recvbuf[0], raw_frame.size);
  printf("\n");
  */

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


/***************************
STORING RECEIVED FRAMES
***************************/


//Parses a raw frame buffer into a FRAME struct, and store it in the link's received queue. Used by end-nodes only.
uint8_t parse_raw_and_store(RAW_FRAME raw, LINK *link)
{
	uint8_t i, j;
	FRAME frame;
	
	//Parse the RAW_FRAME into a structured FRAME
	frame = raw_to_frame(raw);
	free(raw.buf);
	
  //make sure the received queue is not full
  if (link->rqueue_pending == RECV_QUEUE_SIZE)
  {
    printf("Receive Queue is full! Dropping frame...\n");
    return 0;
  }
 

  //Find an empty slot to store the newly received frame, starting from the last sending index
  for (i = link->rqueue_head, j = 0; j < RECV_QUEUE_SIZE; j++)
  {
    //Wrap index i around to the beginning if needed
    if (i >= RECV_QUEUE_SIZE ) i = 0;
	
	//Check if this spot is marked free
	if (link->recv_queue[i].size == 0) break;
	
	//Increment i to check the next slot
	i++;
  }
  
  //Store the frame
  link->recv_queue[i] = frame;
  link->rqueue_pending++;
  
  /*
  printf("*QUEUED:*\n");
  print_frame(link->recv_queue[i]);
  printf("Buffered into recv_queue pos %d\n", i);
  */
  
  return 1;
	
}

FRAME pop_recv_queue(LINK *link)
{
	FRAME retframe;
	FRAME *curhead;
	
	//Make sure the recv queue has pending data first
	if(link->rqueue_pending == 0)
	{
		retframe.size = 0;
		return retframe;
	}
	
	//Copy the current head out of the recv queue
	curhead = &link->recv_queue[link->rqueue_head];
	memcpy(&retframe, curhead, sizeof(FRAME));
	
	//Advances queue head and cleanup
	if (++link->rqueue_head >= RECV_QUEUE_SIZE) 
		link->rqueue_head = 0;
	
	curhead->size = 0;
	link->rqueue_pending--;
	
	
	//Remember to free the payload


	return retframe;
}


uint8_t read_serial(LINK *link)
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
  while(rawframe.size > 0)
  {
    //Parse the frame and store it in the recvd buffer
	parse_raw_and_store(rawframe, link);
    
	//Check if the rbuf contains more complete packets
    proc_buf(NULL, 0, link);
    rawframe = extract_frame_from_rbuf(link);
	frames_received++;
  }	
  
  return frames_received;
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

  
  //Transmit the packet out onto the link
  link->port->write(link->send_queue[i].buf, link->send_queue[i].size);
  
  /*
  printf("\nTransmitting:\n");
  print_frame(raw_to_frame(link->send_queue[i]));
  printf("\n\n");
  */

  
  //cleanup & mark this slot as free
  link->squeue_lastsent = i;
  link->squeue_pending--;
  link->send_queue[i].size = 0;
  free(link->send_queue[i].buf);

  return i;
}


//IMPORTANT: User must free frame.payload MANUALLY when using any of these 3 functions!
uint8_t send_frame(FRAME frame, LINK *link)
{
	return add_to_send_queue(frame_to_raw(frame), link);
}

uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link)
{
	return add_to_send_queue(frame_to_raw(create_frame(src, dst, size, payload)), link);
}

uint8_t create_send_cframe(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link)
{
	return add_to_send_queue(frame_to_raw(create_cframe(src, dst, size, payload)), link);
}



/***************************
ROUTING
***************************/


uint8_t update_rtable_entry(uint8_t id, uint8_t hops, LINK *link)
{
	//Make sure the source id is valid
	if(id == 0 || id >= MAX_ADDRESS)
	{
		printf("ERROR: Attempted to add a routing entry for address 0 (link-ctrl) or %d (broadcast)\n", MAX_ADDRESS);
		return 0;
	}

	//Increment the routing table entry count
	if(link->rtable[id].hops > 0)
		printf("Overwriting existing route entry for node %d\n", id);
	else
		link->rtable_entries++;
	
	//Update the entry
	link->rtable[id].hops = hops;
	link->rtable[id].ticks = 0;
	printf("Updated Routing entry: %d, %d hops\n", id, link->rtable[id].hops);
	
	return 1;
}


/***************************
SENDING
***************************/

uint8_t send_hello_msg(uint8_t my_id, uint8_t dst_id, uint16_t msg_id, LINK *link)
{	
	uint8_t pl_size = LINK_MSG_SIZE + 1 + 2;		//Buffer for preamble + type + identifier
	uchar msg[pl_size];		

	//Copy the preamble string to the payload
	strncpy(msg, PROBE_PREAMBLE, LINK_MSG_SIZE);
	
	//Append my link type after the preamble, and sent time
	switch(link->link_type)
	{
		case GATEWAY:
			msg[LINK_MSG_SIZE] = SWITCH_LINK_SYMBOL;
			break;
			
		case ENDPOINT:
			msg[LINK_MSG_SIZE] = NODE_LINK_SYMBOL;
			break;
		
		//UNKNOWN and other unexpected types
		default:
			msg[LINK_MSG_SIZE] = 0;
	}
	
	//Append the last 2 bytes of the current timestamp to the packet as an identifier
	memcpy(&msg[LINK_MSG_SIZE+1], &msg_id, sizeof(msg_id));
	
	/*
	printf("Probe msg:");
	print_bytes(msg, LINK_MSG_SIZE + 3);
	printf("\n");
	*/
	
	//Create and send out an "HELLO" message
	printf("Sending HELLO\n");
	create_send_cframe(my_id, 0, pl_size, msg, link);

	return 0;
}

uint8_t send_hello(uint8_t my_id, uint8_t dst_id, LINK *link)
{
	uint16_t msg_id;
	
	link->rtable[dst_id].last_ping_sent = millis();
	msg_id = (uint16_t)link->rtable[dst_id].last_ping_sent;
	
	return send_hello_msg(my_id, 0, msg_id, link);
}


uint8_t send_join_msg(uint8_t my_id, LINK *link)
{
	uint8_t pl_size = LINK_MSG_SIZE + 1;		//Buffer for preamble + hops
	uchar msg[pl_size];		
	
	//Copy the preamble string to the payload
	strncpy(msg, JOIN_PREAMBLE, LINK_MSG_SIZE);
	
	//Append the initial hop count as 1
	msg[LINK_MSG_SIZE] = 0x01;

	/*
	print_bytes(msg, LINK_MSG_SIZE + 1);
	printf("\n");
	*/
	
	//Wait a random period (up to 3 second) before sending
	delay(rand() % 3000);
	
	//Create and send out an "HELLO" message
	printf("Sending JOIN\n");
	create_send_cframe(my_id, 0, pl_size, msg, link);
	
	return 0;
}



uint8_t send_leave_msg(uint8_t id, uint8_t reason, LINK *link)
{
	uint8_t pl_size = LINK_MSG_SIZE + 1;		//Buffer for preamble + reason 
	uchar msg[pl_size];		

	//Copy the preamble string to the payload
	strncpy(msg, LEAVE_PREAMBLE, LINK_MSG_SIZE);
	
	//Append leave reason
	msg[LINK_MSG_SIZE] = (uchar)reason;
	
	/*
	printf("Leave msg:");
	print_bytes(msg, LINK_MSG_SIZE + 1);
	printf("\n");
	*/
	
	//Create and send out an "LEAVE" message
	printf("Sending LEAVE for %d\n", id);
	create_send_cframe(id, 0, pl_size, msg, link);

	return 0;
}



uint8_t send_rtble_msg(uint8_t dst, LINK *link)
{
	uint8_t i, j, writeidx;
	
	uint8_t pl_size = LINK_MSG_SIZE + 1 + link->rtable_entries * NODE_LENGTH;
	uchar msg[pl_size];		//Buffer for preamble + entries
	
	//Copy the preamble string to the payload
	strncpy(msg, ROUTING_PREAMBLE, LINK_MSG_SIZE);
	
	//Append the number of routing entries that follows
	msg[LINK_MSG_SIZE] = link->rtable_entries;
	
	//Append each of the node information to the payload
	for(i=0, j=0; i<RTABLE_LENGTH; i++)
	{
		if(link->rtable[i].hops > 0)
		{
			//Increment the write index for the payload
			writeidx = LINK_MSG_SIZE + 1 + NODE_LENGTH*(j++);
			
			//Write the ID
			msg[writeidx] = (uint8_t)i;
			
			//Write and increment the hops
			msg[writeidx + 1] = (uint8_t)(link->rtable[i].hops + 1);
		}
	}
	
	/*
	printf("Written %d entries. Should be %d\n", j, link->rtable_entries);
	print_bytes(msg, pl_size);
	printf("\n");
	*/
	
	//Create and send out an "RTBLE" message
	printf("Sending RTBLE to %d\n", dst);
	create_send_cframe(0, dst, pl_size, msg, link);
	
	return 0;
}


uint8_t send_reqrt_msg(uint8_t dst, LINK *link)
{
	printf("Sending REQRT to %d\n", dst);
	create_send_cframe(link->id, dst, LINK_MSG_SIZE, ROUTING_PREAMBLE, link);
	
	return 0;
}


/***************************
PARSING
***************************/

uint8_t parse_hello_msg(FRAME frame, LINK *link)
{
	uint8_t end_id = frame.src;
	uchar end_type = frame.payload[LINK_MSG_SIZE];
	uint16_t msg_id = *((uint16_t*)(&frame.payload[LINK_MSG_SIZE+1]));
	
	unsigned long recv_time = millis();
	int rtt;
	uint16_t rtable_idx = end_id;
	uint8_t reply = 0;				//0 = nothing, 1 = reply, 2 = resend 
	
	printf("Received PROBE from %d, type: %c , msg_id: %hu, ", end_id, end_type, msg_id);
	
	//call user's handler
	if(hello_handler(end_id) < 0)
		return 0;
	
	//First time hearing from the other end of the link
	if(link->end_link_type == UNKNOWN)
	{
		reply = 2;
		rtable_idx = 0;
		
		//Record the link type at the other end
		switch(end_type)
		{
			//Other end is a switch
			case SWITCH_LINK_SYMBOL:
				printf("Other end is a GATEWAY\n");
				link->end_link_type = GATEWAY;
				break;
			
			//Other end is an endpoint. Add the other end's ID into the routing table.
			case NODE_LINK_SYMBOL:
				printf("Other end is an ENDPOINT\n");
				link->end_link_type = ENDPOINT;
				update_rtable_entry(end_id, 1, link);
				break;
			
			//Unknown or other unsupported link types
			default:
				printf("Other end is UNKNOWN\n");
				link->end_link_type = UNKNOWN;
		}
	}
	
	//Reset the sender's missed tick count
	link->rtable[rtable_idx].

	rtt = recv_time - link->rtable[rtable_idx].last_ping_sent;
	printf("rtt: %d ms\n", rtt);
	
	//Reply to this HELLO message if necessary
	if (reply || (recv_time - link->rtable[rtable_idx].last_ping_recvd) > IGNORE_PING_UNDER)
	{
		printf("Replying to PROBE...\n");
		send_hello(link->id, end_id, link);
	}
	else
		printf("No need to reply to HELLO\n");
	
	link->rtable[rtable_idx].last_ping_recvd = recv_time;
	
	return 1;
}


uint8_t parse_join_msg(FRAME frame, LINK *link)
{
	uint8_t new_id = frame.src;
	uint8_t new_hops = (uint8_t)frame.payload[LINK_MSG_SIZE];
	
	//call user's handler
	if(join_handler(new_id) < 0)
		return 0;
	
	//Add the node's routing information to the table. The same index as its ID is used.
	update_rtable_entry(new_id, new_hops, link);
	printf("Received NJOIN from %d, %d hops\n", new_id, link->rtable[new_id].hops);
	
	//TODO: If switch, forward the packet to everyone else. Implement in the switch code
	
	//Reply with the current routing table. //TODO: Reply all routing tables
	send_rtble_msg(new_id, link);
	
	return 1;
}



uint8_t parse_leave_msg(FRAME frame, LINK *link)
{
	uint8_t leave_id = frame.src;
	uint8_t reason = (uint8_t)frame.payload[LINK_MSG_SIZE];
	
	//call user's handler
	if(leave_handler(leave_id, reason) < 0)
		return 0;
	
	//Remove the node's routing information to the table. The same index as its ID is used.
	update_rtable_entry(leave_id, 0, link);
	printf("Received LEAVE from %u, %u\n", leave_id, reason);
	
	//TODO: If switch, forward the packet to everyone else. Implement in the switch code
	
	return 1;
}


uint8_t parse_rtble_msg(FRAME frame, LINK *link)
{
	uint8_t entries = (uint8_t)frame.payload[LINK_MSG_SIZE];
	uint8_t i, curid, curhops, readidx;
	
	//call user's handler
	if(rtble_handler(frame.src, entries, frame.payload) < 0)
		return 0;
	
	
	//Switches do not parse anyone else's routing table
	if(link->link_type == GATEWAY) 
		return 0;
	
	printf("Received RTBLE with %d entries!\n", entries);
	
	for(i=0; i<entries; i++)
	{
		//Parse the next routing entry out of the message payload
		readidx = LINK_MSG_SIZE + 1 + NODE_LENGTH*i;
		curid = (uint8_t)frame.payload[readidx];
		curhops = (uint8_t)frame.payload[readidx + 1];
		
		//Insert the current entry from the message into the routing table. TODO: Proper support of virtual interfaces
		printf("Parsed rtable update entry: %d, %d hops\n", curid, curhops);
		if(curid != link->id)	
			update_rtable_entry(curid, curhops, link);	
	}

	return 1;
}



uint8_t parse_reqrt_msg(FRAME frame, LINK *link)
{
	//call user's handler
	if(reqrt_handler(frame.src) < 0)
		return 0;
	
	//Reply with the current routing table.
	printf("Received REQRT from %d, %d hops\n", frame.src);
	send_rtble_msg(frame.src, link);
	
	return 1;
}


CMSG_T parse_control_frame(FRAME frame, LINK *link)
{
	//Do not attempt to process a message frame
	if(frame.preamble != CFRAME_PREAMBLE)
	{
		printf("Not a Control frame\n");
		return Invalid_CFrame;
	}
	
	//Call the corresponding processing function depending on the control message in the payload
	if(strncmp(frame.payload, PROBE_PREAMBLE, LINK_MSG_SIZE) == 0)
	{
		printf("Found PROBE message!\n");
		parse_hello_msg(frame, link);
		return Hello_Frame;
	}
	else if(strncmp(frame.payload, JOIN_PREAMBLE, LINK_MSG_SIZE) == 0)
	{
		printf("Found JOIN message!\n");
		parse_join_msg(frame, link);
		return Join_Frame;
	}
	else if(strncmp(frame.payload, ROUTING_PREAMBLE, LINK_MSG_SIZE) == 0)
	{
		printf("Found RTBLE message!\n");
		parse_rtble_msg(frame, link);
		return Rtble_Frame;
	}
	else if(strncmp(frame.payload, REQRT_PREAMBLE, LINK_MSG_SIZE) == 0)
	{
		printf("Found REQRT message!\n");
		parse_reqrt_msg(frame, link);
		return Rtble_Frame;
	}
	else if(strncmp(frame.payload, LEAVE_PREAMBLE, LINK_MSG_SIZE) == 0)
	{
		printf("Found LEAVE message!\n");
		parse_leave_msg(frame, link);
		return Leave_Frame;
	}
	else
	{
		printf("Unknown Control frame\n");
		print_frame(frame);
		return Invalid_CFrame;
	}
		
}



