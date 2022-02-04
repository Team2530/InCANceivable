#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
//#include <FRC_CAN_utils.h>
//##include <FRC_CAN.h>
//#include <mcp_can.h>

#define PIN_STRIP1 11
const int numLEDs=50;
//const int buttonPin = 8;     // the number of the pushbutton pin
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLEDs, PIN_STRIP1, NEO_GRB + NEO_KHZ800);
int stripProgram=0;
int updateStripTime=50; // (milliseconds)

unsigned long myCanId;
unsigned long heartBeatId = 0x01011840L;
// globals for CAN bus 
//const int SPI_CS_PIN = 17; 
extern MCP_CAN CAN;
extern unsigned char CANflagRecv;
extern unsigned char CANlen;
extern unsigned char CANbuf[8];
extern unsigned char canRunning;

// following is the callback function for the interrupt
// in loop() we make a quick check the flag and 
// read if data is available rather than reading 
// here 
//extern void MCP2515_ISR();
//{
//    CANflagRecv=1;
//}



void stripAllOff(unsigned long int now)
{
//  // Serial.println("stripAllOff called");
strip.setBrightness(0);
strip.show();
 // // Serial.println("stripAllOff done");
return;
}

void stripOne(unsigned long int now)
{
  int i=0;
  unsigned long int cycleStart=0;
  unsigned long int milliSecOn=300;
  unsigned long int milliSecTotal=400;
  //// Serial.println("stripOne called");
//if (!cycleStart) {
//  cycleStart=now;
//}
 for (i=0;i<numLEDs;i+=2)
   {
   strip.setPixelColor(i,255,0,0);
   
   }
   //// Serial.println("set red done");
for (i=1;i<numLEDs;i+=2)
   {
    strip.setPixelColor(i,0,0,255);
   }
   //// Serial.println("set blue done");
   strip.setBrightness(64);
   strip.show();
  
   
//      
//   if ((now-cycleStart)>milliSecOn)
//   {
//       strip.setBrightness(32);
//       strip.show();
//   }
//   else
//   { 
//    if ((now-cycleStart)>milliSecTotal)
//    {
//      cycleStart=now;
//      strip.setBrightness(64);
//      strip.show();
//    }
//   }
}

void stripTwo(unsigned long int now)
{
  int i=0;
  unsigned long int cycleStart=0;
  unsigned long int milliSecOn=200;
  unsigned long int milliSecTotal=400;
//if (!cycleStart) 
//  cycleStart=now;
 for (i=0;i<numLEDs;i++)
   {
   strip.setPixelColor(i,0,255,0);
   }
   strip.setBrightness(64);
   strip.show();
   // Serial.println("strip2 called");
   
//   if ((now-cycleStart)>milliSecOn)
 //  {
  //     strip.setBrightness(0);
  //     strip.show();
  // }
//   else
//   { 
//    if ((now-cycleStart)>milliSecTotal)
//    {
//      cycleStart=now;
//      strip.setBrightness(64);
//      strip.show();
//    }
//   }
}

      
void setup() {
  int i;
//  // Serial.begin(115200);
//  while (!// Serial) {
//    ; // wait for // Serial port to connect. Needed for native USB port only
 // }
  // get set up to be on the FRC can bus 
  myCanId=FRC_CAN_init();  // sets speed, extended protocol etc
  //CANConfigureMasks();
  //attachInterrupt(0,MCP2515_ISR,FALLING); // always use interrupt zero -- that's where the MCP2515
  // chips and the arduino wire together 
  InCANceivable_setup();
  // setup the strip led's 
  strip.begin();
  strip.setBrightness(64);  // if you crank the brightness up you pretty much just burn battery 
  // visually it's not a lot different
  for (i=0;i<numLEDs;i++)
  {
  strip.setPixelColor(i,0,255,0);
  }
  strip.show(); // Initialize all pixels to 'off'
  // Serial.println("Setup is done");
  //pinMode(buttonPin, INPUT);
}


void loop() 
{  
  static long int masterClock;
 // // Serial.print(".");
  //// Serial.print("in loop -- CANflagRecv:");
  //// Serial.println(CANflagRecv);
  //if (CANflagRecv)
  if(CAN_MSGAVAIL==CAN.checkReceive())
  { 
//    // Serial.println("");
    unsigned long canId; 
    canId=CAN.getCanId();
    CAN.readMsgBufID(&canId, &CANlen, CANbuf);
    CANflagRecv=0;
    
    //// Serial.println("messageAvailable");
    if (!canRunning) 
    {  
      // at startup there may be a message in the can bus that is garbage 
      // we get here if it's our first message; we don't do any processing 
      // set the flag so we will start processing on the next message
      // this risks losing one message at startup.
      canRunning=1;
      // Serial.println("just set canRunning");
    }   
    else
    {
   // // Serial.println("have packet");  
    // this is the actual work loop where we process canId 
    // there are some special purpose helper functions      
     if (FRC_CAN_isBroadcast(canId))
        { 
          // Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
          //// Serial.println(canId, HEX); 
          //InCANceivable_msg_dump(canId);
          //FRC_CAN_handleBroadcast(canId);
          // this should handle emergency shutdown and heartbeat
          // somehow it's broken.
        }
     else
        { 
          if (FRC_CAN_isRIO(canId)){
           // if (canId==heartBeatId){
             // Serial.println("beep");
           // }
          }
          else
            {
          
          // myCanId is a global that was set in setup()
          //// Serial.println(" -- not a BROADCAST MESSAGE -- ");
          //// Serial.println(canId, HEX); 
          //// Serial.println(myCanId,HEX);
          if (FRC_CAN_isMe(canId,myCanId))
          {
  //         // Serial.println("GOT ONE!");
           // Serial.println(CANlen); 
           if (CANlen==3){
            stripProgram==99; // out of bounds 
            // Serial.println(CANbuf[0]);
            for (int i=0;i<numLEDs;i++){ 
              strip.setPixelColor(i,CANbuf[0],CANbuf[1],CANbuf[2]);
            }
            if (CANbuf[2]>CANbuf[0]){

              unsigned long int outputCanId= FRC_CAN_embed(myCanId, 1, 9);
              CAN.sendMsgBuf(outputCanId,FRC_EXT,4, CANbuf);
            }
             
            strip.setBrightness(64);
            strip.show();
              
            }
           else{ 
           InCANceivable_msg_dump(canId,CANlen,CANbuf);
            stripProgram=(int)CANbuf[0];
            // reset the master clock 
            masterClock=0;
            // Serial.print("strip program: ");
            // Serial.println(stripProgram);
           }
          }
            }
//       else 
//        {
//          unsigned long int mask=0x40000L;
// 
//          char high[5]={0};
//          char low[5]={0};
//          unsigned long int tmp=canId & 0xffffL;
//          //// Serial.println(tmp,HEX);
//          if ((canId & mask)==0){
//          sprintf(low,"%04X",tmp);
//          sprintf(high,"%04X", canId>>16);
//          // Serial.print(high);
//          // Serial.print(",");
//          // Serial.println(low);
//          
////          InCANceivable_msg_dump(canId);
//          }
//        }
//                   
//          //// Serial.println(canId,HEX);
//        }
        }
  }  
  }
//  
//  switch(stripProgram){
//      case 0:
//      stripAllOff(0);
//      break;
//      case 1:
//      stripOne(1);
//      break;
//      case 2:
//      stripTwo(2);
//      break;
//  }
  // now it is possible that while we were handling the last message a new one appeared 
  // if that's the case we want to go to next pass of this (loop) function 
  // otherwise we will actually update the lights 
  
//if (!CANflagRecv) 
//{
//{ 
//  unsigned long int now;
//  now = millis();
// if (now>(masterClock+updateStripTime))
//   {
//    masterClock=now;
//    switch(stripProgram){
//      case 0:
//      stripAllOff(now);
//      break;
//      case 1:
//      stripOne(now);
//      break;
////      case 2:
////      stripTwo(now);
////      break;
//    }
//  // // Serial.println(stripProgram);
//   }
//}
//// Serial.println("loop done");
}
