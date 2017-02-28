#include "packets.h"

FILE serial_stdout;
int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}


void setup(){

   //Setting up serial1 and stdout redirection
   Serial.begin(9600);
   fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
   stdout = &serial_stdout;

}


void loop() {

  uchar* header;
  uchar payload_test[20] = "ABCDEFGHIJKLMNOPQRS";

  /*Testing UMPACKET*/

  UMPACKET testPacket1 = create_umpacket(0xc, 0x6, 0x13, payload_test);
  UMPACKET testPacket2;
  
  printf("Original Packet: \n");
  print_umpacket(testPacket1);
  
  header = umpacket_header_tobuf (&testPacket1);
  print_bytes(header, UMPACKET_HEADER_SIZE);

  printf("\nRe-read packet:\n");
  testPacket2 = buf_to_umpacket(header);
  print_umpacket(testPacket2);
  printf("****************************\n\n");
  free(header);


  /*Testing USPACKET*/

  USPACKET testPacket3 = create_uspacket(0xc, 0x6, rand(), 0x5678, 0x6767, 0x13, payload_test);
  USPACKET testPacket4;

  printf("Original Packet: \n");
  print_uspacket(testPacket3);
  
  header = uspacket_header_tobuf (&testPacket3);
  print_bytes(header, USPACKET_HEADER_SIZE);

  printf("\nRe-read packet:\n");
  testPacket4 = buf_to_uspacket(header);
  print_uspacket(testPacket4);
  printf("****************************\n\n");
  free(header);
  
  
  /*Testing RSPACKET*/
  
  RSPACKET testPacket5 = create_rspacket_data(0xc, 0x6, 0x3039, 0x13, payload_test);
  //RSPACKET testPacket = create_rspacket_ack(0xc, 0x6, 0x3039, 0x13);
  RSPACKET testPacket6;
  
  printf("Original Packet: \n");
  print_rspacket(testPacket5);

  header = rspacket_header_tobuf (&testPacket5);
  print_bytes(header, RSPACKET_HEADER_SIZE);

  printf("\nRe-read packet:\n");
  testPacket6 = buf_to_rspacket(header);
  print_rspacket(testPacket6);
  printf("****************************\n\n");
  free(header);


  while(1);

}
