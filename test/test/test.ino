#include <stdio.h>
#include <stdlib.h>

FILE serial_stdout;

int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}

void setup()
{
   //Setting up serial1 and stdout redirection
   Serial.begin(9600);
   Serial1.begin(9600);
   fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
   stdout = &serial_stdout;

   printf("test\n");

}



void loop() 
{
  size_t bytes;
  char buf[256];

  //read from console
  bytes = Serial.available();
  if(bytes > 0)
  {
    Serial.readBytes(buf, bytes);
    Serial1.write(buf, bytes);
    memset(buf, 0, 256*sizeof(char));
  }


  //read from remote
  bytes = Serial1.available();
  if(bytes > 0)
  {
    Serial1.readBytes(buf, bytes);
    printf("%s", buf);
    memset(buf, 0, 256*sizeof(char));
  }

  
    

}

