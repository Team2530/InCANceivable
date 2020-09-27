#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>
#include <math.h>
extern MCP_CAN CAN;

unsigned long int inCAN_makeBaseID(int devNum)
{
  unsigned long int ret=0;
  unsigned long int tmp=0;
  tmp=devNum;
  ret= INCAN_MASK | tmp;
  return(ret);
}    

unsigned long int  FRC_CAN_init()
{
  int devNum; 
  unsigned long int myCanId;
  pinMode(FRC_DEV_PIN1, INPUT_PULLUP);
  pinMode(FRC_DEV_PIN2, INPUT_PULLUP);
  devNum=digitalRead(FRC_DEV_PIN2) <<1 | digitalRead(FRC_DEV_PIN1) ;
  myCanId=inCAN_makeBaseID(devNum);
  delay(100);
  Serial.println("FRC_CAN_init -- initializing CAN board");

  while (CAN_OK != CAN.begin(FRC_CAN_SPEED))            
    {
      Serial.println("CAN BUS Shield init fail");
      Serial.println(" Init CAN BUS Shield again");
      delay(100);
    }
  
  Serial.println("CAN BUS Shield init ok!");
  return(myCanId);
}

int FRC_CAN_HWmatch(unsigned long int canId, unsigned long  int myId, int *devNum)
{
  int ret;
  ret= ((canId & INCAN_MASK)==(myId & INCAN_MASK));
  *devNum=(int) canId & FRC_DEVNUM_MASK;  // FRC_DEVNUM_SHIFT=0  
  return(ret);
}
 // checks manuf, device id fields returns 1 if match, does not check device number, but updates devNum if pointer is not NULL.

int FRC_CAN_isMe(unsigned long int canId, unsigned long int myId)
{  // like HW match but also requires matching device number.
  int ret;
  unsigned long int myMask=INCAN_MASK || FRC_DEVNUM_MASK;
  ret=((canId & myMask) == (myId & myMask));
  return(ret);
}
int FRC_CAN_isBroadcast(unsigned long int canId)
{
  int ret;
  unsigned long int tmp;
  tmp=canId & ( FRC_DEVICE_MASK | FRC_MANUFACT_MASK );
  //Serial.println(tmp);
  
  ret= (tmp==0);
  // Serial.println(ret);
    return(ret);
}

int FRC_CAN_handleBroadcast(unsigned long int canId)
{
  unsigned long int msgClass;
  int ret=-1;
  // ASSUME that we only get called if this a message from the broadcast device 
  msgClass=canId & FRC_CLASS_MASK;
  if (msgClass==0) 
  {
    unsigned long int msgClindex;
    msgClindex=canId & FRC_CLINDEX_MASK;
    msgClindex=msgClindex>>FRC_CLINDEX_SHIFT;
    ret=msgClindex ;  // demote class 
    if (ret==FRC_BROADCAST_DISABLE)
      {
	Serial.println("FRC CRASH being called");
	FRC_CRASH(0);
      }
    else
      {
	if (ret==FRC_BROADCAST_HEARTBEAT)
	  { 
	    FRC_CAN_heartBeat(1);
	  }
      }
  } 
  return(ret);
}
int FRC_CAN_isRIO(unsigned long int canId)
{ // makes strong assumptions about the ID’s of the control computer
int ret;
 unsigned long int tmp;
 tmp =canId & (FRC_DEVICE_MASK | FRC_MANUFACT_MASK);
 ret=( tmp ==RIO_MASK);
return(ret);
}

void FRC_CAN_extractClass(unsigned long canId, int *apiClass, int *apiIndex)
{
  *apiClass = (int) (canId >>FRC_CLASS_SHIFT) & FRC_CLASS_MASK;
  *apiIndex = (int) (canId >>FRC_CLINDEX_SHIFT) & FRC_CLINDEX_MASK;
}

unsigned long int FRC_CAN_embed(unsigned long baseID, int apiClass, int apiIndex)
{
  unsigned long int ret;
  unsigned long int ltmp;
  // blot out the class and index bits then set them rather than relying on the user to have presented us "clean" baseID
  baseID=baseID & ~((FRC_CLASS_MASK <<FRC_CLASS_SHIFT) | (FRC_CLINDEX_MASK << FRC_CLINDEX_MASK));
  ret = baseID | ((unsigned long int)apiClass << FRC_CLASS_SHIFT) | ((unsigned long int)apiIndex << FRC_CLINDEX_SHIFT);
  return(ret);
}

int HBclock=0;
int FRC_CAN_heartBeat(int reset)
{
  // call this with reset = 1 whenever you see a FRC heartbeat go by;  if reset=0 then int should be the state to set onto the onboard LED allowing you to call this every pass through the loop() function and just set the pin.  (All the clock management is handled internally to allow fast/slow blinks.) There are three time windows here;   1) how long since last HB? if that's more than maxMillis then fast blink;  if less then slow blink.
  int maxMillis=5000;
  int slowMillis=1000;
  int fastMillis=100;  
  int ret;
  if (reset) 
  {  
    HBclock=millis();
  }
  if ((millis()-HBclock)>maxMillis)
    {
    // then not receiving HB resets
    ret=((millis()/fastMillis) % 2); ;
    }
  else
    {
   // blink slowMillis
      ret=((millis()/slowMillis) % 2);
    }
     return(ret);
}
void FRC_CRASH(int pin)
{ // turn on the LED  and go into a loop that never returns.  You must hardware reset or power cycle to get out of this. 
pinMode(LED_BUILTIN,OUTPUT);
 if (pin==0)
{
  pin=LED_BUILTIN;
 }   
digitalWrite(pin,HIGH);
while (1)
{
}
}

