#include <node.h>
#include <uart_stdout.h>
#include <Servo.h>

//GPIO
#define SERVO_PIN       2
#define DEMO_LED_PIN    12
#define ACTIVITY_LED_PIN  13

//Network
#define MY_ID  2

//Others
#define NET_INIT_DELAY 1000
#define LED_PERIOD 2000

LINK *link;
Servo servo;

uint8_t led_on = 1;
uint8_t led_intensity = 128;

uint8_t msg_src = MY_ID;
uint8_t msg_dst = MY_ID;

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


void move_servo(uint8_t servo_pos)
{
  static uint8_t last_pos = 0;

  if (servo_pos > 180 || servo_pos < 0)
    return;

  if (last_pos == servo_pos)
    return;

  //Moves the servo to the new position
  last_pos = servo_pos;
  servo.write(servo_pos);
}

/***/

void mframe_parser(FRAME frame)
{
  uint8_t pos = 90, inten = 128;
  
  if (strncmp(frame.payload, "!MVSV", 5) == 0)
  {
    printf("!MVSV from %u\n", frame.src);
    pos = *((uint8_t*) &frame.payload[5]);
    inten = *((uint8_t*) &frame.payload[6]);

    //Update local devices
    move_servo(pos);
    led_intensity = inten;
    
    if(led_on)  
      analogWrite(DEMO_LED_PIN, led_intensity);
  }
  else if (strncmp(frame.payload, "!TLED", 5) == 0)
  {
    printf("!TLED from %u\n", frame.src);
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

}


//Does not handle peer removal yet?
int8_t led_peer_update(FRAME frame)
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
    my_sucessor = find_successor(MY_ID, link);

    if(my_sucessor == 0)
      my_sucessor = find_predecessor(MY_ID, link);

    msg_src = my_sucessor;
    msg_dst = my_sucessor;

    printf("***TWO PEOPLE!\t\t s:%u p:%u \n", my_sucessor, my_sucessor);

    //Send out initial message
    if(my_sucessor > MY_ID) 
      send_led_msg(1);

    return;
  }

  //3+ nodes

  my_sucessor = find_successor(MY_ID, link);
  my_predecessor = find_predecessor(MY_ID, link);
  
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



void setup()
{
  //Initialize GPIOs
  pinMode(DEMO_LED_PIN, OUTPUT);      //Demo LED
  pinMode(ACTIVITY_LED_PIN, OUTPUT);  //Link Activity LED
  pinMode(SERVO_PIN, OUTPUT);         //Servo PWM
  servo.attach(SERVO_PIN);            //Initialize Servos

  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  //Setup the network
  delay(NET_INIT_DELAY);                  //Give the switch a bit of time to initialize
  link = node_init(MY_ID, mframe_parser);
  set_join_handler(led_peer_update);
  set_leave_handler(led_peer_update);
  set_rtble_handler(led_peer_update);
  send_join_msg(link->id, link);

  //Initial
  //send_led_msg(msg_dst, 255);
}


void loop()
{
  net_task(0);
  send_led_msg(0);
}




