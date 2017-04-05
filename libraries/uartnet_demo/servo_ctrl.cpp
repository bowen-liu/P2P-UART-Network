#include "servo_ctrl.h"

//Declared in led_cycler.cpp, only needed for dynamic brightness
extern uint8_t led_intensity;
extern uint8_t led_on;
#define DEMO_LED_PIN    12

static LINK *link;
static Servo servo;

//Used by workers and master
void move_servo(uint8_t servo_pos)
{
  static uint8_t last_pos = 0;

  if (servo_pos > 180 || servo_pos < 0)  return;

  if (last_pos == servo_pos) return;

  //Moves the servo to the new position
  last_pos = servo_pos;
  servo.write(servo_pos);
}

//Used by master only
void joystick_servo()
{
  static uint8_t last_pos = 90;            //Keeps track of the POS from the previous iteration
  static uint8_t last_inten = 128;

  uint8_t servo_pos;
  uint8_t led_inten;
  uchar msg[7] = "!MVSV";

  //Read joystick X axis and map it to a degree between 10 to 170
  servo_pos = map(analogRead(JOYSTICK_X_PIN), 0, 1023, 10, 170);

  //Read joystick Y axis and map it to a intensity between 0 and 255
  led_inten = map(analogRead(JOYSTICK_Y_PIN), 0, 1023, 0, 255);

  //No need to send POS/Intensity if either hasn't changed
  if (servo_pos != last_pos)
    last_pos = servo_pos;
  else if (led_inten != last_inten)
    last_inten = led_inten;
  else if (servo_pos == last_pos && led_inten == last_inten)
    return;

  //Broadcast the new POS

  //strncpy(msg, "!MVSV", 5);
  msg[5] = (uchar)servo_pos;
  msg[6] = (uchar)led_inten;
  create_send_frame(link->id, MAX_ADDRESS, 7, msg, link);

  //Update local devices
  move_servo(servo_pos);
  led_intensity = led_inten;

  if(led_on)
    analogWrite(DEMO_LED_PIN, led_intensity);

  /*
    printf("Servo pos: %u\n", servo_pos);
    printf("LED inten: %u\n", led_inten);
    print_bytes(msg, 7);
    printf("\n");
  */

}

void parse_mvsv_cmd(FRAME frame)
{
	uint8_t pos = 90, inten = 128;
  
  /*
  if (strncmp(frame.payload, "!MVSV", 5) != 0)
	return
	*/
	  
    
    pos = *((uint8_t*) &frame.payload[5]);
    inten = *((uint8_t*) &frame.payload[6]);

    //Update local devices
    move_servo(pos);
    led_intensity = inten;
    
    if(led_on)  
      analogWrite(DEMO_LED_PIN, led_intensity);
}


void servo_ctrl_init(LINK *my_link)
{
	link = my_link;
	servo.attach(SERVO_PIN);            //Initialize Servos
}


