#include "routing.h"

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


uint8_t find_successor(uint8_t id, LINK *link)
{
	int i;
	
	if(link->rtable_entries == 0)
		return 0;
	
	if(id == MAX_ADDRESS)
		return 0;
	
	//Find the lowest valid ID starting from the end of the rtable
	for(i = id; i < RTABLE_LENGTH; i++)
	{
		if(link->rtable[i].hops > 0)
		{
			printf("Sucessor ID: %u\n", i);
			return i;
		}
	}
	
	//Didn't find any nodes in this range?
	if(i == MAX_ADDRESS)
	{
		printf("Successor ID: 0 (cannot find)\n");
		return 0;
	}
}


uint8_t find_predecessor(uint8_t id, LINK *link)
{
	int i;
	
	if(link->rtable_entries <= 0)
		return 0;
	
	if(id >= MAX_ADDRESS)
		id = MAX_ADDRESS - 1;
	
	//Find the highest valid ID starting from the end of the rtable
	for(i = id; i > 0; i--)
	{
		if(link->rtable[i].hops > 0)
		{
			printf("HIGHEST ID: %u\n", i);
			break;
		}
	}
	
	//Will return 0 if no nodes are found in the range
	return i;
}




/***************************
SENDING
***************************/

uint8_t send_hello_msg(uint8_t my_id, uint8_t dst_id, LINK *link)
{	
	uint8_t pl_size = LINK_MSG_SIZE + 1;		//Buffer for preamble + type 
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
	
	link->rtable[dst_id].last_ping_sent = millis();
	
	return send_hello_msg(my_id, 0, link);
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
	
	unsigned long recv_time = millis();
	int rtt;
	uint16_t rtable_idx = end_id;
	uint8_t reply = 0;				//0 = nothing, 1 = reply, 2 = resend 
	
	printf("Received PROBE from %d, type: %c ", end_id, end_type);
	
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
	
	
	//call user's handler
	hello_handler(frame);

	
	return 1;
}


uint8_t parse_join_msg(FRAME frame, LINK *link)
{
	uint8_t new_id = frame.src;
	uint8_t new_hops = (uint8_t)frame.payload[LINK_MSG_SIZE];
	
	//Add the node's routing information to the table. The same index as its ID is used.
	update_rtable_entry(new_id, new_hops, link);
	printf("Received NJOIN from %d, %d hops\n", new_id, link->rtable[new_id].hops);
	
	//TODO: If switch, forward the packet to everyone else. Implement in the switch code
	//Reply with the current routing table If I'm the switch. 

	
	//call user's handler
	join_handler(frame);
	
	return 1;
}



uint8_t parse_leave_msg(FRAME frame, LINK *link)
{
	uint8_t leave_id = frame.src;
	uint8_t reason = (uint8_t)frame.payload[LINK_MSG_SIZE];
	
	
	//Remove the node's routing information to the table. The same index as its ID is used.
	update_rtable_entry(leave_id, 0, link);
	printf("Received LEAVE from %u, %u\n", leave_id, reason);
	
	//TODO: If switch, forward the packet to everyone else. Implement in the switch code
	
	//call user's handler
	leave_handler(frame);
	
	return 1;
}


uint8_t parse_rtble_msg(FRAME frame, LINK *link)
{
	uint8_t entries = (uint8_t)frame.payload[LINK_MSG_SIZE];
	uint8_t i, curid, curhops, readidx;		
	
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
	
	//Call User's handler
	rtble_handler(frame);
	
	return 1;
}



uint8_t parse_reqrt_msg(FRAME frame, LINK *link)
{
	
	//Reply with the current routing table.
	printf("Received REQRT from %d, %d hops\n", frame.src);
	send_rtble_msg(frame.src, link);
	
	//call user's handler
	reqrt_handler(frame);
	
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



