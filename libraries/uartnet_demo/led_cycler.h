#ifndef _UARTNET_LEDCYCLE_DEMOH_
#define _UARTNET_LEDCYCLE_DEMOH_

#include "Arduino.h"

#include <frame.h>
#include <link.h>

//GPIOs
#define ACTIVITY_LED_PIN  13		//Not used
#define DEMO_LED_PIN    12

//Other
#define LED_INIT_DELAY 3000
#define LED_PERIOD 2000

void led_cycler_init(uint8_t id, LINK *my_link);
void send_led_msg(int8_t tosend);
int8_t led_peer_update();
void toggle_led();

#endif