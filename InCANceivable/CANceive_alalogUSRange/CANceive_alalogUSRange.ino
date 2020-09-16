#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>


// globals for CAN bus 
const int SPI_CS_PIN = 17; 
MCP_CAN CAN(SPI_CS_PIN);
unsigned char CANflagRecv=0;
unsigned char CANlen=0;
unsigned char CANbuf[8];
char canRunning=0;
// this is the callback function for the interrupt
// in loop() we make a quick check the flag and 
// read if data is available rather than reading 
// here 
void MCP2515_ISR()
{
    CANflagRecv=1;
}
void CANConfigureMasks()
{
 // Nasty hardwre knowledge required here (read the MCP2515 chip data sheet)  
 // there are two filter banks;  filter bank 0 has 2 masks 
 // filter bank 1 has 4 masks.  I *think* that design idea was that you could 
 // actually buffer two messages in hardware  -- the example code available online 
 // does not implement that.  We really only need to listen to two families of ID's
 // the zero address for broadcast and the robot controller (RoboRio) 
 // The way this works it given an ID on the wire, apply the mask, then see if what is
 // left matches a filter. 
  CAN.init_Mask(0,FRC_EXT,FRC_DEVICE_MASK | FRC_MANUFACT_MASK); // accept based on DEVICE AND MANUFACTURER
  CAN.init_Mask(1,FRC_EXT,0);  // turn off input filter bank 1 entirely
  CAN.init_Filt(0,FRC_EXT,0); // accept the broadcast device
  CAN.init_Filt(1,FRC_EXT,RIO_MASK); // accept data from the RIO
}

// globals for analog ultrasound 
const int analogUSPinOut=4;
const int analogUSPinIn=5;
int analogUSMaxRange=90;  // in mm
#define analogUSBufSz 50
float analogUSHistory[analogUSBufSz];
char analogUSIndex=0;
int numAverage=10; 
// 
int initAnalogUSRange(int outPin, int inPin)
{
// pretty simple, we just set the pin for digital i/o 
  pinMode(outPin,OUTPUT);
  pinMode(inPin,INPUT);
}
//
float averageUSRange()
{ 
  // USRange data is in a ring buffer so we have to a little math to figure out what to average

float sum=0;
int i=0;
int ii;
for (i=0; i<analogUSBufSz; i++)
{
   ii=analogUSIndex-i+1;
   if (ii<0)
   {
   ii=ii+analogUSBufSz;
  } 
  sum=sum+analogUSHistory[ii];
}
  return(sum/analogUSBufSz);
}
 
float getAnalogUSRange(int outPin, int inPin)
//  the analog ultrasonic range finder is rated for
// ~4cm to 4m.  the sensor is really simple you send out a US for a while
//  then poll in a very tight loop listening until the pulse goes away
//  If the distances are large the time can be relatively long so we also 
//  enable a clipping  -- for example if the distance is more than analogUSMaxRange
//  we don't really care how much more... 
//  We will assume 343,300 mm/s for speed of sound in air (ignore humidity and altitude)
//  That is 0.3433 mm / micro sec, but we weill be measuring out-and-back so use half the number of tics  
{
float microSecTomm = 0.343 * 0.5  ;  // the 0.5 is since microsecs we measure are round trip.
float ret=0.0;
float echoTime;

unsigned long int maxMicroSec;
maxMicroSec=ceil(analogUSMaxRange/microSecTomm);
digitalWrite(outPin, HIGH);
delayMicroseconds(10); 
digitalWrite(outPin, LOW);
echoTime = pulseIn(inPin, HIGH,maxMicroSec);    
 //use the pulsein command to see how long it takes for the
 //pulse to bounce back to the sensor
ret=echoTime*microSecTomm;
return(ret);
} 

 
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  FRC_CAN_init();
  CANConfigureMasks();
 
 initAnalogUSRange(analogUSPinOut,analogUSPinIn); 
}
unsigned long int lastDumpMillis=0;
unsigned long int millisBetweenDumps=200;  

void loop() 
{
  float USrange;
  unsigned long int now;
  if (CANflagRecv)
{
  if (canRunning) // see note in else block
  {
   unsigned long canId = CAN.getCanId();
   CAN.readMsgBuf(&CANlen,CANbuf);  
   CANflagRecv=0;  // as soon as we've moved our data out reset that flag; 
   // the idea being that this could keep open the possibility for interrupt
   // to catch another bus msg
     if (FRC_CAN_isBroadcast(canId))
        { 
          Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
          Serial.println(canId, HEX); 
          FRC_CAN_handleBroadcast(canId);
        }
        else
        {  // not a broadcast message -- probably from the RIO
        // for now we are just dumping the message -- intelligent parsing needs to be done here
        Serial.println("-----------------------------");
        Serial.println("Get data from ID: ");
        //Serial.println(canId, BIN);
        for (unsigned long int mask = 0x80000000; mask; mask >>= 1) {
           Serial.print(mask&canId?'1':'0');
        }
        Serial.println();
        Serial.println("10987654321098765432109876543210");
        Serial.println(" 3         2         1");
        Serial.print("Length of data received: ");
        Serial.println(CANlen); 
        for(int i = 0; i<CANlen; i++)    // print the data
        {
            Serial.print(CANbuf[i], HEX);
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
  else
  { 
    // at startup there may be a message in the can bus that is garbage 
    // we get here if it's our first message; we didn't do any processing 
    // set the flag so we will start processing new messages
    canRunning=1;
  }
} 
// depending on how long it took us to handle the messages, it is (theoretically) possible 
// that another message was received.  It's more important to handle that than it is to read the sensors and send data 
// if CANflagRecv has been reset then skip processing and let loop() start over 
if (!CANflagRecv) 
{ // read sensors, updating internal data;  if enough time past make output message(s)
  now = millis();
  if (now > (lastDumpMillis+millisBetweenDumps))
  // if enough time has elapsed since the last time we wrote data then emit data
  {
    float *buf=(float*)CANbuf; // make type casting obvious
    unsigned long int messageID;
    Serial.println("DUMP");
    messageID=FRC_CAN_embed(INCAN_MASK,INCAN_CL_ANAUS, INCAN_ANAUS_DATA);
    buf[0]=averageUSRange();
    buf[1]=analogUSHistory[analogUSIndex];
    CAN.sendMsgBuf(messageID,FRC_EXT, CANlen, CANbuf);
    lastDumpMillis=now;
  } // end of send data messages 
  // handle the sensors 
  analogUSHistory[analogUSIndex]=getAnalogUSRange(analogUSPinOut,analogUSPinIn);
  analogUSIndex++;
  analogUSIndex= analogUSIndex % analogUSBufSz;
} // end of send data messages and update sensor data 
  
} // end of loop
