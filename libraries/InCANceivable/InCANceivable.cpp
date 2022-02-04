#include <InCANceivable.h>
#include <mcp_can.h>

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

int verbose=1;
extern unsigned char CANflagRecv;

void MCP2515_ISR()
{
  Serial.println("interrupt");
  CANflagRecv=1;
}

void InCANceivable_setup()
{
  //  const int CAN_INT_PIN=0;
  CANConfigureMasks();
  //  attachInterrupt(0, &MCP2515_ISR, FALLING); 

  // Serial.println("Done attaching interrupt");
} 

void CANConfigureMasks()  // this should have mycanid as an argument

{
 // Nasty hardwre knowledge required here (read the MCP2515 chip data sheet)  
 // there are two filter banks;  filter bank 0 has 2 masks 
 // filter bank 1 has 4 masks.  I *think* that design idea was that you could 
 // actually buffer two messages in hardware  -- the example code available online 
 // does not implement that.  We really only need to listen to three families of ID's
 // the zero address for broadcast
 // the robot controller (RoboRio) 
 // and stuff with our ID's 
 // The way this works it given an ID on the wire, apply the mask, then see
 //  if what is left matches a filter. 
 //  MASK will reject anything without a good  chance of being valid FRC canId 
 //  then we will filter out everything that isn't broadcast, rio or InCan
 // make sure we are pasing unsigned long
  unsigned long int device_manufact_mask=(FRC_DEVICE_MASK<<FRC_DEVICE_SHIFT) | (FRC_MANUFACT_MASK<<FRC_MANUFACT_SHIFT);  
  //Serial.print(" device mfg mask :" );
  //Serial.println(device_manufact_mask,HEX);  
  CAN.init_Mask(0,FRC_EXT,device_manufact_mask);
  //CAN.init_Mask(0,FRC_EXT,0);
  //CAN.init_Filt(0,FRC_EXT,0);
  CAN.init_Mask(1,FRC_EXT,device_manufact_mask);
  CAN.init_Filt(0,FRC_EXT,RIO_MASK); 
  CAN.init_Filt(1,FRC_EXT,INCAN_MASK);
  CAN.init_Filt(2,FRC_EXT,0); 
  // the INCAN_MASK is a little bit loose -- if you have two incanceivables on the 
  // bus this will leak across them -- example, say one is writing messages
  // about a set of sensor
  // and the other is reading messages to set LED's 
  // the one reading messages will have process the other guy's messages 
  // ("process" here probably means read the mcp chipset and reject based on 
  // not exactly matching the canId.  Arduino's could update the filters to be
  // more device specific 
}

void InCANceivable_msg_dump(unsigned long canId, int len, unsigned  char *buf)
{
    // just dumping the message 
    Serial.println("-----------------------------");
    Serial.println("CAN Bus ID binary dump: ");
    for (unsigned long int mask = 0x80000000; mask; mask >>= 1) {
      Serial.print(mask&canId?'1':'0');
    }
    Serial.println();
    Serial.println("10987654321098765432109876543210");
    Serial.println(" 3         2         1");
    Serial.print("Length of data received: ");
    Serial.println(CANlen); 
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


char messageCheck(unsigned long int *canId)
{
  *canId = 0;
  int ret=0;
  if(CANflagRecv)
    {
      if (canRunning==0)
	{
	  canRunning=1; // discards the first message in CAN subsystem
	}
	else
	  { 
	    *canId=CAN.getCanId();
	    CAN.readMsgBuf(&CANlen,CANbuf);
	    CANflagRecv=0; 
	    if (FRC_CAN_isBroadcast(*canId))
	      {
                if (verbose)
		  {
		    Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
		    Serial.println(*canId, HEX);
		  } 
		FRC_CAN_handleBroadcast(*canId);
	      }
	    else
	      {
                ret=1;
	      }
	  }
    }
  return(ret);
}
