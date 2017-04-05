#include "led_cycler.h"


static LINK *link;
static uint8_t my_id;

uint8_t led_on = 1;
uint8_t led_intensity = 128;

static uint8_t msg_src = my_id;
static uint8_t msg_dst = my_id;


//Tosend: -1 = reset; 0 = update LED only; 1 = send message
void send_led_msg(int8_t tosend)
{
  static unsigned long start_time = 0;
  unsigned long current = millis();

  //Cancel pending send?
  if(tosend < 0)
  {
    start_time = 0;
    return;
  }

  //Do we need to toggle yet?
  if (tosend == 0 && start_time == 0)
    return;

  //Start the timer for new send request
  if(tosend > 0)
    start_time = millis();

  if(start_time > 0 && current - start_time < LED_PERIOD)
    return;
  else if(start_time > 0 && current - start_time >= LED_PERIOD)
  {   
    create_send_frame(link->id, msg_dst, 5, "!TLED", link);
    start_time = 0;
  }
}

//Does not handle peer removal yet?
int8_t led_peer_update()
{
  uint8_t my_sucessor;
  uint8_t my_predecessor;


  //Reset any pending send timers
  send_led_msg(-1);
  
  //Reset LED to off state
  led_on = 0;
  analogWrite(DEMO_LED_PIN, 0);


  //I'm alone
  if(link->rtable_entries == 0)
  {
    printf("*LED: I'm alone\n");
    return;
  }

  //Special case with 2 nodes: src and dst are both the same
  if(link->rtable_entries == 1)
  {
    //Who's the other node?
    my_sucessor = find_successor(my_id, link);

    if(my_sucessor == 0)
      my_sucessor = find_predecessor(my_id, link);

    msg_src = my_sucessor;
    msg_dst = my_sucessor;

    printf("***TWO PEOPLE!\t\t s:%u p:%u \n", my_sucessor, my_sucessor);

    //Send out initial message
    if(my_sucessor > my_id) 
      send_led_msg(1);

    return;
  }

  //3+ nodes

  my_sucessor = find_successor(my_id, link);
  my_predecessor = find_predecessor(my_id, link);
  
  printf("***LED ROUTING UPDATE!\t\t s:%u p:%u \n", my_sucessor, my_predecessor);

  //I'm alone
  if(my_sucessor == 0 && my_predecessor == 0)
  {
    printf("*LED: I'm alone\n");
    return;
  }

  //I'm the highest
  if(my_sucessor == 0)
  {
    printf("*LED: I'm highest\n");
    msg_src = my_predecessor;
    msg_dst = find_successor(0, link);
  }
  //I'm the lowest
  else if(my_predecessor == 0)
  {
    printf("*LED: I'm lowest\n");
    msg_src = find_predecessor(MAX_ADDRESS, link);
    msg_dst = my_sucessor;

    //Send out a toggle message if I was alone
    send_led_msg(1);
  }
  else
  {
    printf("*LED: I'm in the middle somewhere\n");
    msg_src = my_predecessor;
    msg_dst = my_sucessor;
  }

  printf("***NEW\t\t s:%u p:%u \n", msg_src, msg_dst);

  return 0;
}


void toggle_led()
{
	if (led_on)
    {
      analogWrite(DEMO_LED_PIN, 0);
      led_on = 0;
    }
    else
    {
      analogWrite(DEMO_LED_PIN, led_intensity);
      led_on = 1;
    }
	send_led_msg(1);
}

void led_cycler_init(uint8_t id, LINK *my_link)
{
	my_id = id;
	link = my_link;
}
