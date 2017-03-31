#include "node.h"
#include <uart_stdout.h>
#include <Servo.h>

//GPIO
#define SERVO_PIN       2
#define DEMO_LED_PIN    12
#define ACTIVITY_LED_PIN  13

//Network
#define MY_ID  2
#define MSG_SRC_EXPECTED  1
#define MSG_DST  3

//Others
#define NET_INIT_DELAY 1000
#define LED_PERIOD 2000

LINK *link;
Servo servo;

uint8_t led_on = 1;
uint8_t led_intensity = 128;


void send_led_msg(uint8_t tosend)
{
  static unsigned long start_time = 0;
  unsigned long current = millis();

  //Do we need to toggle yet?
  if (!tosend && start_time == 0)
    return;

  if(tosend)
  {
    start_time = millis();
    return;
  }

  if(start_time > 0 && current - start_time < LED_PERIOD)
    return;
  else if(start_time > 0 && current - start_time >= LED_PERIOD)
  {
    create_send_frame(link->id, MSG_DST, 5, "!TLED", link);
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
  send_join_msg(link->id, link);

  //Initial
  //send_led_msg(msg_dst, 255);
}


void loop()
{
  net_task(0);
  send_led_msg(0);
}




