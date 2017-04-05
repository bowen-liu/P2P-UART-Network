/*Routing related functions and delcarations*/

#ifndef _UARTNET_LINK_ROUTINGH_
#define _UARTNET_LINK_ROUTINGH_

#include <link_common.h>
#include "cframe_callback.h"


/*******************************
Control Frames
*******************************/

typedef enum {Invalid_CFrame = 0, Hello_Frame, Join_Frame, Rtble_Frame, Leave_Frame} CMSG_T;

//Link Layer messages
#define LINK_MSG_SIZE               	6
#define PROBE_PREAMBLE			((const char*) "!HELLO")
#define JOIN_PREAMBLE          	((const char*) "!NJOIN")
#define REQRT_PREAMBLE			((const char*) "!REQRT")
#define ROUTING_PREAMBLE		((const char*) "!RTBLE")
#define LEAVE_PREAMBLE          ((const char*) "!LEAVE")

//For PROBE messages
#define SWITCH_LINK_SYMBOL				's'
#define NODE_LINK_SYMBOL				'n'

//For LEAVE messages (Leave Reason)
#define	UNEXPECTED_LEAVE				0x00
#define NOREASON_LEAVE					0x01


/*******************************
Active Monitorings
*******************************/

//Timer settings for active monitoring
//#define TICK_MS					10000000	//In MS; 10 seconds per tick
//#define PING_TICKS				30			//Ping every 5 minutes
//#define PING_TICKS_THRESHOLD	33		
//#define IGNORE_PING_UNDER		5000			//currently in MS, to be changed to TICKS


#define TICK_MS					5000000		//In MS; 3 seconds per tick
#define PING_TICKS				3			
#define PING_TICKS_THRESHOLD	5	
#define IGNORE_PING_UNDER		10000		//currently in MS, to be changed to TICKS


/*******************************
User Functions?
*******************************/
CMSG_T parse_control_frame(FRAME frame, LINK *link);
uint8_t find_successor(uint8_t id, LINK *link);
uint8_t find_predecessor(uint8_t id, LINK *link);


/*******************************
Sending Control Frames
*******************************/

uint8_t send_hello(uint8_t my_id, uint8_t dst_id, LINK *link);
//uint8_t send_hello_msg(uint8_t my_id, uint8_t dst_id, LINK *link);
uint8_t send_join_msg(uint8_t my_id, LINK *link);
uint8_t send_leave_msg(uint8_t id, uint8_t reason, LINK *link);
uint8_t send_rtble_msg(uint8_t dst, LINK *link);
uint8_t send_reqrt_msg(uint8_t dst, LINK *link);

#endif