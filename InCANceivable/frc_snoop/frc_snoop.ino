#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>
#include <Wire.h>  // prereq for the 8452Q
#include <SparkFun_MMA8452Q.h>   

MMA8452Q accel;                



const int SPI_CS_PIN = 17; 
MCP_CAN CAN(SPI_CS_PIN);

unsigned long int DeviceNumber=0xf;
unsigned long int DeviceMask=0;
void setup() {
   Serial.begin(115200);
   delay(100);
/*   Serial.println("Starting setup");

    while (CAN_OK != CAN.begin(FRC_CAN_SPEED))            
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
    */
    FRC_CAN_init();
    // set device number here based on pins 10 11 jumper status 
    // can use the internal resistors and pull down.  
    // (see example sketch)
    DeviceMask=INCAN_MASK | DeviceNumber;      

}
unsigned char len=0;
unsigned char buf[8]={ 0,0,0,0,0,0,0,0};
unsigned int *pinData=(unsigned int*)buf;
unsigned int discardedFirst=0;
void loop() {
  // get the heartBeat status and dump it on the built-in led
  //  digitalWrite(LED_BUILTIN,FRC_CAN_heartBeat(0));
  // we will do fast polling  -- not interrupt driven -- apparently 20ms is about as fast as you can run interrupts  
  if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        unsigned long canId = CAN.getCanId();
        if (discardedFirst==0)
          discardedFirst=1;
          else
          {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf    
      
        // make sure we are not getting broadcast message
        if (FRC_CAN_isBroadcast(canId))
        { 
          Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
          Serial.println(canId, HEX);
          delay(5000);
         // FRC_CAN_handleBroadcast(canId);
        }
        
        Serial.println("-----------------------------");
        Serial.println("Get data from ID: ");
        //Serial.println(canId, BIN);
        for (unsigned long int mask = 0x80000000; mask; mask >>= 1) {
           Serial.print(mask&canId?'1':'0');
        }
        Serial.println();
        Serial.println("10987654321098765432109876543210");
        Serial.println(" 3         2         1");  
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }
        Serial.println();
        Serial.println("FRC-specific deconstruction");
        Serial.print("Device type: ");
        Serial.print((canId >> FRC_DEVICE_SHIFT) & FRC_DEVICE_MASK, HEX);
        Serial.print(" Manufacurer Number: ");
        Serial.println((canId >> FRC_MANUFACT_SHIFT) & FRC_MANUFACT_MASK, HEX); 
        Serial.print("ClassNum: ");
        Serial.print((canId >> FRC_CLASS_SHIFT) & FRC_CLASS_MASK, HEX);
        Serial.print(" ClassIndex: ");
        Serial.print((canId >> FRC_CLINDEX_SHIFT) & FRC_CLINDEX_MASK, HEX);
        Serial.print(" Device Instance Number: ");
        Serial.println((canId >> FRC_DEVNUM_SHIFT) & FRC_DEVNUM_MASK ,HEX);
        
          }
    }

}
