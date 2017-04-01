#ifndef _UARTNET_NODEH_
#define _UARTNET_NODEH_

//#include "link_layer/link.h"
#include <frame.h>
#include <link.h>

#include <TimerOne.h>

//LINK link;

LINK* node_init(uint8_t id, void (*mparser)(FRAME));
void net_task(uint8_t continuous);


#endif
