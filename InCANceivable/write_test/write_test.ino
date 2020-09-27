#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>
// that should have brought in the mcp_can and SPI headers too

extern MCP_CAN CAN;
// FRC_CAN_Utils.cpp took care of setting this up at compile time
// we just use it here. 

//unsigned long int DeviceNumber=0;
unsigned long int DeviceMask=0;

void setup() {
   Serial.begin(115200);
   delay(3000);  // it can take a long time to get the serial open
   Serial.println("Starting setup");
   DeviceMask=FRC_CAN_init();
   // we hold onto DeviceMask since it gets pushed into every data message we will send 
   Serial.print("Done setting up CAN.... Device mask is " ); 
   Serial.println(DeviceMask,HEX);
   Serial.println("We will simply echo the stat of pins 8 and 9 over CAN bus as");
   Serial.println("a counter");
    Serial.println("Ground Pin 8 and/or 9 to change data");
    pinMode(8,INPUT_PULLUP);
    pinMode(9,INPUT_PULLUP);
}
short loopCount=0;
unsigned char buf[8]={ 0,0,0,0,0,0,0,0};
unsigned int *pinData=(unsigned int*)buf;
// there's a little compiler magic there -- buf is an array of 8 "chars" or single byte data
// we want to use is as integers so we define pinData which points to an array of integers
// they just happen to be the same physical memory. 

void loop() {
  
  unsigned long int messageID;
  loopCount++;

  messageID=FRC_CAN_embed(DeviceMask,INCAN_CL_PIN, INCAN_PIN_DATA);
  // FRC_CAN_embed takes the DeviceMask we got from initializing the system
  // and jams in a couple of bits of information.  Remember that each device has a 
  // Type, Manufacturer, message class, and message class index.  DeviceMask contained Type and Manufacturer
  // stuffed into the right places.  We just filled in the other two things ... 
  // Available classes come from FRC_CAN.h and for each class there is a list of classIndex values. All our 
  // CLasses and Class Index values are defined in symbols beginning with INCAN_ 
  // INCAN_CL_PIN means data came from reading a pin on the arduino board.  INCAN_PIN_DATA means that the 
  // data block contains information about the state of the pins.
  Serial.println(messageID,HEX);
  pinData[0]=0;
  pinData[1]=0;
  pinData[2]=0;
  pinData[3]=0;
  pinData[0]=pinData[0] | digitalRead(8)<<8 | digitalRead(9)<<9;
  pinData[2]=pinData[2] | 1<<8 |  1<<9;
  // Note, we applied some knowledge here; pinData is unsigned int which on arduino leonardo is 2 bytes
  // pinData points to a block of memory that is 8 bytes long; so we can access elemnts pinData[0] through
  // pinData[3];  We zeroed them out then checked pins 8 and 9, turning bits 8 and 9 of pinData[0] on if 
  // appropriate. 
  
  CAN.sendMsgBuf(messageID,FRC_EXT, 8, buf);
  // anti-climactic -- after we set up our messageID and filled in the data buffer we just send a message
  // Note:  always always use the FRC_EXT and 8 for FRC robots. 
  delay(1000);
  // we wait a second and then do it again -- can run faster but if you have two canBed on a bench
  // then you might set the one with this code up to write and then run something to read on the other one
  // (like frc_snoop). 

}
