#include "cframe_callback.h"


//Set the pointers

void set_message_handler(void (*handler_ptr)(FRAME))
{
	message_handler = handler_ptr;
}

void set_hello_handler(void (*handler_ptr)(uint8_t))
{
	hello_handler = handler_ptr;
}

void set_join_handler(void (*handler_ptr)(uint8_t))
{
	join_handler = handler_ptr;
}

void set_leave_handler(void (*handler_ptr)(uint8_t, uint8_t))
{
	leave_handler = handler_ptr;
}

void set_rtble_handler(void (*handler_ptr)(uint8_t, uint8_t, uchar*))
{
	rtble_handler = handler_ptr;
}

void set_reqrt_handler(void (*handler_ptr)(uint8_t))
{
	reqrt_handler = handler_ptr;
}


//Default handlers

void message_handler_default(uint8_t src_id)
{
	return;
}

void hello_handler_default(uint8_t src_id)
{
	return;
}

void join_handler_default(uint8_t src_id)
{
	return;
}

void leave_handler_default(uint8_t src_id)
{
	return;
}

void rtble_handler_handler(uint8_t src_id, uint8_t rtble_length, uchar* raw_payload)
{
	return;
}

void reqrt_handler_handler(uint8_t src_id)
{
	return;
}


//Initialize all handlers with default handlers

void (*message_handler)(FRAME) 					= message_handler_default; 
void (*hello_handler)(uint8_t) 					= hello_handler_default;
void (*join_handler)(uint8_t)					= join_handler_default;
void (*leave_handler)(uint8_t, uint8_t)			= leave_handler_default;
void (*rtble_handler)(uint8_t, uint8_t, uchar*)	= rtble_handler_handler;
void (*reqrt_handler)(uint8_t)					= reqrt_handler_handler;


