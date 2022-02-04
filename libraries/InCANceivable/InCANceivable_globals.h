#ifndef INCANCEIVABLE_GLOBALS_INCLUDED 
#define INCANCEIVABLE_GLOBALS_INCLUDED 

unsigned char CANflagRecv=0;
unsigned char CANlen=0;
unsigned char CANbuf[8];
unsigned char canRunning=0;  // we start this out with "not running" so that the first 
// message we receive is actually dropped -- possible bug in the underlying CANbed libraries puts garbage in the 
// first message .... 
#endif // INCANCEIVABLE_GLOBALS_INCLUDED
