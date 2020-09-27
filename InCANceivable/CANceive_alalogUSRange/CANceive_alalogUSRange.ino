#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>
#include <InCANceivable.h>
// this is a bit more sophisticated than (for example) the write_test.ino or frc_snoop.ino in this package
// Those both had clocks that were just free running and polled for data as fast as possible. This uses InCANceivable
// functions to configure the CAN bus to "do the right thing" for most (all?) practical FRC reasons.

extern MCP_CAN CAN;  // FRC_CAN_utils provides this
extern unsigned char canRunning = 0; // InCANceivable provides this
extern unsigned char CANflagRecv = 0; // InCANceivable provides this too
unsigned char CANlen = 8;
unsigned char CANbuf[8];



// globals for analog ultrasound
// the analog ultrasound is noisy so we build up a small number of measurements and average them
// the last "analogUSBufSz" measurements are kept in analogUSHistory and output is "numAverage" of the most
// recent measurements.
// Note that the analogUSHistory is used as a "ring buffer" and analogUSIndex tells us where we have most recently
// written
// Note that we have assumed you have one and only one analogUSRange finder per unit.  That is not a necessary constraint
// but generalizing makes the code heavier.
const int analogUSPinOut = 6; // goes to hardware trigger pin
const int analogUSPinIn = 5; // goes to hardware echo pin
int analogUSMaxRange = 1000; // in mm
int analogUSMinRange = 40; // in mm
#define analogUSBufSz 50
float analogUSHistory[analogUSBufSz];
unsigned int analogUSIndex = 0;
int numAverage = 10;
//
int initAnalogUSRange(int outPin, int inPin)
{
  // pretty simple, we just set the pins for digital i and o
  // to emit ultrasound you need an output pin
  // to listen for changes in the echo you need an input pin
  pinMode(outPin, OUTPUT);
  pinMode(inPin, INPUT);
}
//
float averageUSRange()
{
  // USRange data is in a ring buffer so we have to a little math to figure out what to average
  float sum = 0;
  int i = 0;
  // the way we are managing the counter is that the analogUSIndex is the next slot to be filled
  // so it's not valid data --- we want the "numAverage" bins just behind analogUSIndex 
  int ringIndex=analogUSIndex-numAverage-1;

  for (i = 0; i < numAverage; i++)
  {
    int tmpIndex=ringIndex;
    if (tmpIndex<0)
    { 
      tmpIndex=tmpIndex+analogUSBufSz;
      ringIndex++;
    }
    else
    { 
      ringIndex++;
      ringIndex=ringIndex % analogUSBufSz;      
    }
    sum= sum + analogUSHistory[tmpIndex];
    //Serial.println(ringIndex);
  }
  //Serial.println(sum);
  return (sum / numAverage);
}

float getAnalogUSRange(int outPin, int inPin)
// Get a value from the range finder -- this is at the hardware level, not a smoothed average etc
// The returned value is what you would stuff into analogUSHistory[] buffer.
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
  float ret = 0.0;
  unsigned long echoTime;
  unsigned long int maxMicroSec;
  maxMicroSec = ceil(analogUSMaxRange / microSecTomm);
  digitalWrite(outPin, LOW);
  delayMicroseconds(2);
  digitalWrite(outPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outPin, LOW);
  echoTime = pulseIn(inPin, HIGH, maxMicroSec);
  if (echoTime == 0)
  {
    ret = analogUSMaxRange;
  }
  else
  {

    // echoTime = pulseIn(inPin, HIGH);
    //use the pulsein arduino function to see how long it takes for the
    //pulse to bounce back to the sensor
    // pulseIn is a dead polling loop -- this effectively halts the arduino from doing anything else
    // until it's done or maxMicroSec have passed.
    //Serial.println(echoTime);
    ret = echoTime * microSecTomm;
    if (ret<analogUSMinRange)
    ret=analogUSMinRange;
    // Serial.println(ret);
  }
  return (ret);
}
// End of stuff to handle the data from the analog US range finder
unsigned long int DeviceMask = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100); // allow serial to get going so that subsequent functions can scream if there are problems.
  DeviceMask = FRC_CAN_init();
  CANConfigureMasks();
  initAnalogUSRange(analogUSPinOut, analogUSPinIn);
}

// we will drop data on the CANbus targetting a message every millisBetweenDump (millis= milliseconds)
// We could make millisBetweenDumps really small, but keep in mind, we need to NOT flood the CAN bus killing off
// other control messages
unsigned long int lastDumpMillis = 0;
unsigned long int millisBetweenDumps = 200;
long int millisInWriteLoop=0;
long int millisInLoop=0;
void loop()
{
  float USrange;
  unsigned long int now;
  millisInLoop=millisInLoop-millis();
  if (CANflagRecv)
  {
    if (canRunning == 0)
    {
      // we have a message but it's the first one -- it's quite probably garbage
      // read the buffer and set canRunning=1
      CAN.readMsgBuf(&CANlen, CANbuf);
      canRunning = 1;
    }
    else
    {
      // normal operation  -- we got a message -- we have set up the CAN chipset to only receive messages
      // from broadcast or ROBOrio
      unsigned long canId = CAN.getCanId();
      CAN.readMsgBuf(&CANlen, CANbuf);
      CANflagRecv = 0; // as soon as we've moved our data out reset the  flag;
      // the idea being that this could keep open the possibility for interrupt
      // to catch another bus msg which we may want to prioritize above doing our "day job" of 
      // reading sensor(s) and occasionally sending data out.
      
      if (FRC_CAN_isBroadcast(canId))
      {
        //Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
        //Serial.println(canId, HEX);
        FRC_CAN_handleBroadcast(canId);
        // as of 20200926 handleBroadcast is pretty limited -- it mostly respects the "STOP" command, but that functionality 
        // may be expanded in the future. 
      }
      else
      { 
        InCANceivable_msg_dump(canId); 
        // We could do something with the message here ; for now we are just printing it to the terminal
        // This is just toy code focused on reading sensor, doing a little data massage (averaging), and writing messages
      }
    }
  }
  // depending on how long it took us to handle the messages, it is (theoretically) possible
  // that another message was received.  Assume it's more important to handle that than it is
  // to read the sensors and send data. So make a quick check that CANflagRecv is still set to false
  // -- if it's false then read the sensor data,  if it's true don't do anything else start a new
  // pass through loop() function.
  if (!CANflagRecv)
  { // read sensors, updating internal data every time ; AND if enough time past make output message(s)
    // handle the sensors
    analogUSHistory[analogUSIndex] = getAnalogUSRange(analogUSPinOut, analogUSPinIn);
    analogUSIndex = analogUSIndex+1;
    analogUSIndex = analogUSIndex % analogUSBufSz; // ring buffer index wrap around
    
    now = millis();
    if (now > (lastDumpMillis + millisBetweenDumps))
    {
      millisInWriteLoop=millisInWriteLoop-millis();
      //  enough time has elapsed since the last time emitted data then emit data now
      float *buf = (float*)CANbuf; // make type casting obvious
      unsigned long int messageID;
      //      Serial.println("Dumping message data on CANBus");
      messageID = FRC_CAN_embed(DeviceMask, INCAN_CL_ANAUS, INCAN_ANAUS_DATA);
      buf[0] = averageUSRange(); 
      buf[1] = analogUSHistory[analogUSIndex];
      CAN.sendMsgBuf(messageID, FRC_EXT, CANlen, CANbuf);
      //Serial.println(buf[1]);
      //Serial.println(buf[0]);
      // Educational Note: if you uncomment either of lines above and aren't injecting any other 
      // print's then you can use the SerialPlotter in the ArduinoIDE to see the data stream 
      // You might want to do that so you get a feel for how noisy the data is.  
      // For example,  buf[0] is an average over the last "numAverage" bins (numAverage is defined above)
      // buf[1] is just the last measurement and it's almost certainly "noisier" than the average.
      // the downside of averaging is that it is averaging over time -- so if the physical distance is changing rapidly 
      // the average will not keep up.  HOWEVER, we only hit this code every "millisBetweenDumps" so that also limits the 
      // speed of changes you can actually see.  There is some rudimentary instrumentation in the code to estimate the 
      // fraction of time spent actually measuring data vs dumping data.     
      lastDumpMillis = now; 
      // note that we use the "now" from before when we started writing -- if writing took a long time (or a 
      // wildy varying time, then using the "now" helps insure we start at the right time for the next loop 
      // and don't accumulate a lag   
      millisInWriteLoop=millisInWriteLoop+millis();
    } // end of send data messages
  } // end of send data messages and update sensor data
  millisInLoop=millisInLoop+millis();
  // Serial.println(millisInWriteLoop);
  // Serial.println(float(millisInWriteLoop)/float(millisInLoop));
 // 20200926 -- checked by uncommenting the above code:   millisInWriteLoop is rarely updated (e.g. we don't spend a 
 // lot of time in loop writing data out 
 // the fraction of time we spend writing data out is about 2%.  If you uncomment printing and are running a fast 
 // serial connection (115k baud here) it doesnt' seem to matter much.  If we turned it down to say 300baud it would probably
 // matter (exercise left to the reader).   
} // end of loop
