#ifndef _UARTNET_LINK_CFRAME_CALLBACK_H_
#define _UARTNET_LINK_CALLBACK_H_

#include "link.h"


//Function Pointers for control message handlers
extern void (*message_handler)(FRAME); 
extern void (*hello_handler)(uint8_t);
extern void (*join_handler)(uint8_t);
extern void (*leave_handler)(uint8_t, uint8_t);
extern void (*rtble_handler)(uint8_t, uint8_t, uchar*);
extern void (*reqrt_handler)(uint8_t);


//Function to change the default handlers with a user defined handler
void set_message_handler(void (*handler_ptr)(FRAME));
void set_hello_handler(void (*handler_ptr)(uint8_t));
void set_join_handler(void (*handler_ptr)(uint8_t));
void set_leave_handler(void (*handler_ptr)(uint8_t, uint8_t));
void set_rtble_handler(void (*handler_ptr)(uint8_t, uint8_t, uchar*));
void set_reqrt_handler(void (*handler_ptr)(uint8_t));





#endif