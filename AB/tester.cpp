#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

//RF24 radio(7, 8); // CSN, CE

const int pinCE = 7; 
const int pinCSN = 8;

RF24 wirelessSPI(pinCE, pinCSN);
 
const uint64_t pAddress = 0xB00B1E5000LL;            



void setup()  
{
  Serial.begin(9600);   
  printf_begin();       
  wirelessSPI.begin();          
  wirelessSPI.setAutoAck(1);          
  wirelessSPI.enableAckPayload();             
  wirelessSPI.setRetries(5,15);                 
  wirelessSPI.openWritingPipe(pAddress);        
  wirelessSPI.stopListening();
  wirelessSPI.printDetails();                  

}


void loop()  
{  
  
 
}