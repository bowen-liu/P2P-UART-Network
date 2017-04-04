#ifndef _UARTNET_LINK_CFRAME_CALLBACK_H_
#define _UARTNET_LINK_CALLBACK_H_

#include "link.h"


//Function Pointers for control message handlers
extern int8_t (*message_handler)(FRAME); 
extern int8_t (*hello_handler)	(uint8_t);
extern int8_t (*join_handler)	(uint8_t);
extern int8_t (*leave_handler)	(uint8_t, uint8_t);
extern int8_t (*rtble_handler)	(uint8_t, uint8_t, uchar*);
extern int8_t (*reqrt_handler)	(uint8_t);


//Function to change the default handlers with a user defined handler
void set_message_handler(int8_t (*handler_ptr)(FRAME));
void set_hello_handler(int8_t (*handler_ptr)(uint8_t));
void set_join_handler(int8_t (*handler_ptr)(uint8_t));
void set_leave_handler(int8_t (*handler_ptr)(uint8_t, uint8_t));
void set_rtble_handler(int8_t (*handler_ptr)(uint8_t, uint8_t, uchar*));
void set_reqrt_handler(int8_t (*handler_ptr)(uint8_t));





#endif