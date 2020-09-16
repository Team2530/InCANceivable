#ifndef FRC_CAN_UTILS
#define FRC_CAN_UTILS 
#include <FRC_CAN.h>


// The FRC message spec can let us have up to 4 devices on the bus -- to keep
// them from walking on each other you have to set the device number by 
// grounding some pins. I assume you use pins 10 and 11 for this purpose
#define FRC_DEV_PIN1 10 
#define FRC_DEV_PIN2 11 
// these are pulled high by the internal resistor;   
// 10 and 11 were chose because they are in close proximity to ground on the
// CANBed board  (11 can be grounded wih a standard jumper off and old 
// computer or hard drive))

unsigned long int inCAN_makeBaseID(int devNum);    
// FRC CAN messages muddle the ID of the device with the data payload 
// this function handles making a compliant ID --  in pratice you set this up and remember the baseID then pass that into a function later that sticks data into the data block to make a message you can throw on the bus.
 
unsigned long int  FRC_CAN_init();  // may read jumpers to get device number and call makeBaseID, returns a canId with everything but the api class and api instance set.  

int FRC_CAN_HWmatch(unsigned long int canID, unsigned long  int myID, int *devNum); // checks manuf, device id fields returns 1 if match, does not check device number, but updates devNum if pointer is not NULL. 
// Usage:given canID, a message your read from the CAN bus and myID is returned from FRC_CAN_init  -- return true if the canID is for the same type of hardware.  If you pass in devNum as a valid memory address it will be filled in with the device number.  

int FRC_CAN_isMe(unsigned long int canId, unsigned long int myID); 
// like HW match but also requires matching device number.   Say we found a
//message on the CANBus and we want to see if is perhaps addressed to us  
// example:  if (FRC_CAN_isMe(canMessage,myID) then *parse the message*

int FRC_CAN_isBroadcast(unsigned long int canId);
// FRC defines device type =0 and manufacturer=0 for broacast messages
// if (FRC_CAN_isBroadcast(canMessage)) then *parse the message*
// NOTE YOU DON'T HAVE A CHOICE -- YOU MUST PARSE broadcast messages as they 
// are where the emergency shutdown is triggered  -- see FRC_CAN_handleBroadcast()

int FRC_CAN_handleBroadcast(unsigned long int canId);
// By and large we don't want user space to be doing things with the broadcast messages
// there's not a lot of "optional" stuff --  especially we want to crash on disable etc
// putting it here means higher level code doesn't have to remember to handle this case
// if this returns, the api index is the return value

int FRC_CAN_isRIO(unsigned long int canId); 
// makes strong assumptions about the ID’s of the control computer
// DID THIS MESSAGE COME FROM A ROBO RIO?

void FRC_CAN_extractClass(unsigned long canId, int *apiClass, int *apiIndex);
// FRC Can spec defines message classes -- they are packed into the canId message, this just pulls them out. 

unsigned long int FRC_CAN_embed(unsigned long baseID,int apiClass,int apiIndex); // opposite of FRC_CAN_extractClass 


// The CanBed has an onboard LED; the management of that LED status is handled internal to this function -- if the light is on solid it means the we haven't received a heartbeat in a while.  
int FRC_CAN_heartBeat(int reset);   // call this with reset = 1 whenever you see a FRC heartbeat go by;  if reset=0 then integer return value should be the state to set onto the onboard LED allowing you to call this every pass through the loop() function and just set the pin.  (All the clock management is handled internally to allow fast/slow blinks.)   

void FRC_CRASH(int pin); // turn on the LED  and go into a loop that never returns.  You must hardware reset or power cycle to get out of this.   This should be called if the CAN bus message for emergency shutdown message is found.

#endif
