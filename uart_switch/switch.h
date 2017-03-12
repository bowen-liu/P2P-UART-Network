#ifndef _UARTNET_SWITCHH_
#define _UARTNET_SWITCHH_

//#include "link_layer/link.h"
#include <frame.h>
#include <link.h>


#define TOTAL_LINKS         1
#define RECV_BUFFER_SIZE    2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#define FLUSH_THRESHOLD     RECV_BUFFER_SIZE * 0.5





#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif
