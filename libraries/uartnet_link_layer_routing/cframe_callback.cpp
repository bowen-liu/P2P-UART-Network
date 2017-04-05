#include "cframe_callback.h"


//Set the pointers

void set_message_handler(void (*handler_ptr)(FRAME))
{
	message_handler = handler_ptr;
}

void set_hello_handler(void (*handler_ptr)(FRAME))
{
	hello_handler = handler_ptr;
}

void set_join_handler(void (*handler_ptr)(FRAME))
{
	join_handler = handler_ptr;
}

void set_leave_handler(void (*handler_ptr)(FRAME))
{
	leave_handler = handler_ptr;
}

void set_rtble_handler(void (*handler_ptr)(FRAME))
{
	rtble_handler = handler_ptr;
}

void set_reqrt_handler(void (*handler_ptr)(FRAME))
{
	reqrt_handler = handler_ptr;
}


//Default handlers

void message_handler_default(FRAME frame)
{
	return 0;
}

void hello_handler_default(FRAME frame)
{
	return 0;
}

void join_handler_default(FRAME frame)
{
	return 0;
}

void leave_handler_default(FRAME frame)
{
	return 0;
}

void rtble_handler_default(FRAME frame)
{
	return 0;
}

void reqrt_handler_default(FRAME frame)
{
	return 0;
}


//Initialize all handlers with default handlers

void (*message_handler)(FRAME) 					= message_handler_default; 
void (*hello_handler)(FRAME) 					= hello_handler_default;
void (*join_handler)(FRAME)					= join_handler_default;
void (*leave_handler)(FRAME)			= leave_handler_default;
void (*rtble_handler)(FRAME)	= rtble_handler_default;
void (*reqrt_handler)(FRAME)					= reqrt_handler_default;


