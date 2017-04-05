#ifndef _UARTNET_SERVOCTRL_DEMOH_
#define _UARTNET_SERVOCTRL_DEMOH_

#include "Arduino.h"
#include <Servo.h>

#include <frame.h>
#include <link.h>


//GPIOs
#define JOYSTICK_X_PIN  A0
#define JOYSTICK_Y_PIN  A1
#define SERVO_PIN       2

//Common
void servo_ctrl_init(LINK *my_link);
void move_servo(uint8_t servo_pos);


//For master only
void joystick_servo();
		

//For slave only
void parse_mvsv_cmd(FRAME frame);
			



#endif