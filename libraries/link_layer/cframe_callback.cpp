#include "cframe_callback.h"


//Set the pointers

void set_message_handler(int8_t (*handler_ptr)(FRAME))
{
	message_handler = handler_ptr;
}

void set_hello_handler(int8_t (*handler_ptr)(uint8_t))
{
	hello_handler = handler_ptr;
}

void set_join_handler(int8_t (*handler_ptr)(uint8_t))
{
	join_handler = handler_ptr;
}

void set_leave_handler(int8_t (*handler_ptr)(uint8_t, uint8_t))
{
	leave_handler = handler_ptr;
}

void set_rtble_handler(int8_t (*handler_ptr)(uint8_t, uint8_t, uchar*))
{
	rtble_handler = handler_ptr;
}

void set_reqrt_handler(int8_t (*handler_ptr)(uint8_t))
{
	reqrt_handler = handler_ptr;
}


//Default handlers

void message_handler_default(uint8_t src_id)
{
	return 0;
}

void hello_handler_default(uint8_t src_id)
{
	return 0;
}

void join_handler_default(uint8_t src_id)
{
	return 0;
}

void leave_handler_default(uint8_t src_id)
{
	return 0;
}

void rtble_handler_handler(uint8_t src_id, uint8_t rtble_length, uchar* raw_payload)
{
	return 0;
}

void reqrt_handler_handler(uint8_t src_id)
{
	return 0;
}


//Initialize all handlers with default handlers

int8_t (*message_handler)(FRAME) 					= message_handler_default; 
int8_t (*hello_handler)(uint8_t) 					= hello_handler_default;
int8_t (*join_handler)(uint8_t)						= join_handler_default;
int8_t (*leave_handler)(uint8_t, uint8_t)			= leave_handler_default;
int8_t (*rtble_handler)(uint8_t, uint8_t, uchar*)	= rtble_handler_handler;
int8_t (*reqrt_handler)(uint8_t)					= reqrt_handler_handler;


