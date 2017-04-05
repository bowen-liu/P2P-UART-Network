#ifndef _UARTNET_NODEH_
#define _UARTNET_NODEH_

//#include "link_layer/link.h"
#include <link.h>


//LINK link;

LINK* node_init(uint8_t id, int8_t (*mparser)(FRAME));
void net_task(uint8_t continuous);


#endif
