#include <node.h>
#include <uart_stdout.h>

#include <led_cycler.h>
#include <servo_ctrl.h>

//Network
#define MY_ID  3

//Others
#define NET_INIT_DELAY 1000

LINK *link;

//Declared in led_cycler.cpp
extern uint8_t led_intensity;
extern uint8_t led_on;

//Overriding the message frame handler, so we can parse commands specific to our application
void mframe_parser(FRAME frame)
{
  if (strncmp(frame.payload, "!MVSV", 5) == 0)
  {
    printf("!MVSV from %u\n", frame.src);
    parse_mvsv_cmd(frame);
  }
  else if (strncmp(frame.payload, "!TLED", 5) == 0)
  {
    printf("!TLED from %u\n", frame.src);
    toggle_led();
  }
}

//Callback for JOIN, LEAVE, and RTBLE. Needed by LED cycler
void route_update_parser(FRAME frame)
{
  led_peer_update();
}


void setup()
{
  //Initialize GPIOs
  pinMode(DEMO_LED_PIN, OUTPUT);      //Demo LED
  pinMode(ACTIVITY_LED_PIN, OUTPUT);  //Link Activity LED
  pinMode(SERVO_PIN, OUTPUT);         //Servo PWM

  //Setting up serial1 and stdout redirection
  stdout_uart_init();

  //Setup the network
  delay(NET_INIT_DELAY);                  //Give the switch a bit of time to initialize
  link = node_init(MY_ID, mframe_parser);
  set_join_handler(route_update_parser);
  set_leave_handler(route_update_parser);
  set_rtble_handler(route_update_parser);
  send_join_msg(link->id, link);

  //start the LED blinking cycle 
  servo_ctrl_init(link);
  led_cycler_init(MY_ID, link);
}


void loop()
{
  net_task(0);
  send_led_msg(0);
}




