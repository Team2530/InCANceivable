#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>
// that should have brought in the mcp_can and SPI headers too

const int SPI_CS_PIN = 17; 
MCP_CAN CAN(SPI_CS_PIN);

unsigned long int DeviceNumber=0;
unsigned long int DeviceMask=0;
void setup() {
   Serial.begin(115200);
   delay(3000);  // it can take a long time to get the serial open
   Serial.println("Starting setup");
   DeviceMask=FRC_CAN_init();
    //while (CAN_OK != CAN.begin(FRC_CAN_SPEED))            
/*    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
*/   
//    Serial.println("CAN BUS Shield init ok!");
    
    Serial.println("Ground Pin 8 and/or 9 to change data");
    pinMode(8,INPUT_PULLUP);
    pinMode(9,INPUT_PULLUP);
    Serial.println("other set up goes here");
}
short loopCount=0;
unsigned char buf[8]={ 0,0,0,0,0,0,0,0};
unsigned int *pinData=(unsigned int*)buf;

void loop() {
  
  unsigned long int messageID;
  loopCount++;
//  messageID=DeviceMask | INCAN_CL_PIN << FRC_CLASS_SHIFT  | INCAN_PIN_DATA << FRC_CLINDEX_SHIFT;
  messageID=FRC_CAN_embed(DeviceMask,INCAN_CL_PIN, INCAN_PIN_DATA);
  Serial.println(messageID,HEX);
  pinData[0]=0;
  pinData[1]=0;
  pinData[2]=0;
  pinData[3]=0;
  pinData[0]=pinData[0] | digitalRead(8)<<8 | digitalRead(9)<<9;
  pinData[2]=pinData[2] | 1<<8 |  1<<9;
  
  CAN.sendMsgBuf(messageID,FRC_EXT, 8, buf);
  
//  Serial.println("foo");
  delay(1000);
  // put your main code here, to run repeatedly:

}
