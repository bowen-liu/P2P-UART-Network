#ifndef _UARTNET_LINK_CFRAME_CALLBACK_H_
#define _UARTNET_LINK_CALLBACK_H_

#include <link.h>



//Function Pointers for control message handlers
extern void (*message_handler)	(FRAME); 
extern void (*hello_handler)	(FRAME);
extern void (*join_handler)		(FRAME);
extern void (*leave_handler)	(FRAME);
extern void (*rtble_handler)	(FRAME);
extern void (*reqrt_handler)	(FRAME);


//Function to change the default handlers with a user defined handler
void set_message_handler(void (*handler_ptr)(FRAME));
void set_hello_handler(void (*handler_ptr)(FRAME));
void set_join_handler(void (*handler_ptr)(FRAME));
void set_leave_handler(void (*handler_ptr)(FRAME));
void set_rtble_handler(void (*handler_ptr)(FRAME));
void set_reqrt_handler(void (*handler_ptr)(FRAME));





#endif