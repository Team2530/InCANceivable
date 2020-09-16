#ifndef INCANCEIVABLE_INCLUDED 
#define INCANCEIVABLE_INCLUDED 
#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <MCP_CAN.h>
/* The InCANceivable project has three software layers to put data onto the CAN bus in an FRC compatible way.  
 InCANceivable is the most abstract;  it relies on functions in FRC_CAN_utils.h which in turn rely on FRC_CAN.h.
 
 The idea here is to "do the right thing for most cases" ;  you can always use the FRC_CAN_utils and FRC_CAN 
 directly. 
 
 Most of the variables defined here are global and we don't make any effort to protect them from being clobbered
*/ 
const int SPI_CS_PIN = 17; 

extern unsigned char CANflagRecv;
extern unsigned char CANlen;
extern unsigned char CANbuf[8];
extern unsigned char canRunning;

// this is the callback function for the interrupt
// in loop() we make a quick check the flag and 
// read if data is available rather than reading 
// here 

void MCP2515_ISR();

void InCANceivable_setup();
void CANConfigureMasks();
void InCANceivable_msg_dump(unsigned long canId);
char messageCheck(unsigned long int *canId); // returns true if there is a message
// canId, as well as CANlen and CANbuf will have been updated 
// messageCheck will consume broadcast messages calling FRC_CAN_handleBroadcast() for those 
// the intent is that if(messageCheck) then the caller has a message to process  
// this should in turn handle heartbeat timers etc.  

#endif // INCANCEIVABLE_INCLUDED
