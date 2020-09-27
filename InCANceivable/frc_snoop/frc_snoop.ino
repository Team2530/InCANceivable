#include <FRC_CAN_utils.h>
#include <FRC_CAN.h>
#include <mcp_can.h>



// TOP LEVEL DISCUSSION
// frc_snoop is meant to read FRC CANbus messages and dump them on the serial terminal over USB
// Assumes: you have an FRC robot, a CANbed from SeeedStudios, and a computer that can read the USB serial stream
// This serves three purposes:  1) lets you see what's chattering on the CANbus (but you can do that with "official"
// FRC WPIlib software so of limited use)   2) It non-invasively verifies
// the various code libraries in the InCANceivable projects are configured correctly to work with FRC can bus
// specs; and 3) its intended to be heavily commented example code.
// As with all Arduino top level sketches:  you can define global stuff that can be specified outside of
// actually running code outside of functions.  Then at run time the function setup() is called once and then loop()
// is called over and over forever.


extern MCP_CAN CAN;
// CAN is an object with a few classes to read/write/poll etc
// The MCP_CAN class is brought in through  mcp_can.h and mcp_can.cpp
// The PARTICULAR INSTANCE is created in InCANceivable.h/cpp where
// the it is configured to use the correct internal pin for interfacing the
// arduino and CAN components on the SeeedStudio CANbed.
// Since someone else is defining it, we attach our notion ot CAN to an extern-ally
// defined one using the extern keyword.



void setup() {
  Serial.begin(115200);
  // start up communications on the USB so we can dump things
  // to a host computer ....
  delay(100);
  // Sometimes it's good to wait just a little bit for the Serial connection to stabilize
  // YMMV but delay is in milliseconds so this is a 0.1 sec delay at startup

  FRC_CAN_init();
  // this is a function that sets up the CAN device to talk on FRC CAN bus
  // sets speed, reads pins 10 and 11 status to determine a device number
  // they are encoded into the low two bits of a device number (so you can have 4
  // on the CANbus at once)
  // Specifically: if you do nothing to pins 10 and 11 then the float high
  // and your device number would be 3;  ground only pin 10 to pull it low for device 2
  // ground only pin 11 to get device 1; ground both 10 and 11 to get device 0
  // It may be possible to change this in software prior to calling FRC_CAN_init() by setting the pins to
  // be pulled down internally.  As of 2020, the spec allows device numbers up to 2^6.  We are only supporting 4.
  // It should be straightforward to allow more units if you want.

  // IF your Arduino were doing something more interesting like reading out a sensor,  you would
  // 1) define any necessary storage and parameters outside of setup() and loop(), init the sensor(s)
  // here, and read the sensors out inside loop.  See other examples for how to do that.
  // Note that the arduino can juggle many sensors (basically as long as it can read and manage data fast
  // enough to meet your needs the hardware limits are modest.
  Serial.println('setup() done');
  delay(5000);

}

// we now define some variables that are global so we can share them back and forth between our code and library
// code to send data in and out over the CAN bus
unsigned char len = 0;
unsigned char buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
unsigned int *pinData = (unsigned int*)buf;


// It's important that get good messages;  it's possible that the CAN bus adapter chips can start up with garbage
// in them so we define a variable that says "we have discarded the first message" and hence are good to go.
// Set it 0 (false) here since we clearly haven't discarded the first message before we've read anything at all.
// You might worry about missing a message, but them's the breaks.  Practically, the FRC CAN bus has lot of chatter
// it's unlikely that you care about a single missing message at power up-- or rather you should write code that isn't
// so picky.   N.B. InCANceivable should NEVER EVER be used to control any actuator directly.  That is for safety.
// The main control computer needs to be able to turn everything off.  The intent is that InCANceivable can read sensors
// interface with the CANbus to report sensor values and maybe control "bling" like LED light strings.

unsigned int discardedFirst = 0;
unsigned int loopCount = 0;

// An aside: inside the InCANceivable library there are some utility functions that will
// take care of keeping track of a "heartbeat" variable -- FRC robot can bus get's occasional signals that are
// broadcast to all devices to tell them that the main computer is alive and enabled.  If the library functions
// are seeing the heartbeat, the heartbeat variable goes on and off slowly -- all good.  If heartbeats are missed, the variable goes
// on and off more rapidly. There is also a FRC robot can bus message for emergency stop --- if InCANceivable sees
// one of those the heartbeat variable is set on and the library goes into an infinite loop.  The only way out of
// the emergency stop is a hardware reboot.  The intended use is:  inside loop() you read the heartbeat variable and
// set the onboard LED to that value.  This helps you know if the system is working.  It's not necessary here since
// frc_snoop assumes you are hooking your canBED up to a computer to see what's on the bus.


// now we define our loop
// we look to see if there is a message to process; if it's not the first message, then we deconstruct the CAN bus message
// header and print out the data, check the heartbeat variable and use it to update the onboard LED.
// Most of the ugliness inside the loop is creating printed output.
// The pedagogical value is is probably that a CAN bus message includes two things:  1) the CANid which is packed
// full of information about "who sent the message and what kind of messge is it?" and 2) a very small tidbit of
// data.
void loop() {
  loopCount = ++loopCount;

  // get the heartBeat status and dump it on the built-in led (Compiler know the number for LED_BUILTIN based
  // on the system you are compiling for.  Calling FRC_CAN_heartbeat(0) says "give me the current value".
  digitalWrite(LED_BUILTIN, FRC_CAN_heartBeat(0));
  // There are two ways we could look for messages on the CAN bus -- the CAN adapter chips can "raise an interrupt"
  // that triggers running some other code or we can "poll" ourselves.  There's a lot of "machinery" to make that work and
  // apparently 20ms is about as fast as you can run interrupts. With polling we just check to see if a message is ready
  // -- if no message loop ends quickly and the system runs loop again.  Polling is of limited value if you
  // need to do multiple tasks etc.  But here we have a limited scope so KISS.
  //


  if (CAN_MSGAVAIL == CAN.checkReceive())           // check if data coming
  {
    unsigned long canId = CAN.getCanId();
    //loopCount=1;
    // Aside: every message on the CAN bus has a CAN Id.  If you come from a point to point networking backgroudn
    // where every network packet has a "to" and a "from" this may feel weird.  The CAN ID can contain the "to"
    // address or the "from" address. E.g. A message intended to set a motor contoller variable would have
    // the canId for the motor controller.  But a message *from* a motor controller indicting current draw would
    // also have the canID for the motor controller.
    // For this example, we just get any message, unpack the canId and print info out.

    if (discardedFirst == 0)
    {
      discardedFirst = 1;
      CAN.readMsgBuf(&len, buf); // have to read the buffer so we kickstart reading more!
    }
    else
    {
      // so far all we know is that there's a message, we haven't transferred the data from the
      // hardware buffer in the CAN chip set over to the Arduino's memory.  So read the message.
      CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
      // For FRC CAN the buffer and is small by definition, and hence len is short too.
      // There are device specific messages and broadcast messages.  Broadcast messages should
      // be attended to by all devices on the can bus -- this for example provides the mechanism for emergency
      // shutdown mesages to be sent.
      // Check for broadcast (FRC_CAN library provides a utility function for this)
      if (FRC_CAN_isBroadcast(canId))
      {
        Serial.println("--BROADCAST MESSAGE BEING HANDLED --");
        Serial.println(canId, HEX);
        delay(5000);  // this is not quite proper -- but frc_snoop is a development tool.
        // there's a chance the next function call will never return if the message is SYSTEM_HALT
        // we put the delay in so the serial line can drain before possibly crashing.  Uncomment if you
        // debugging behavior system halt messages.
        FRC_CAN_handleBroadcast(canId);
        // another utility function ... inside handleBroadcast
        // the message is checked -- if it's a "crash and die" message this function will never return
        // if it's a hearbeat message it'll register that a heartbeat has been seen
        Serial.println("---BROADCAST MESSAGE HANDLING DONE ---");
      }
      // What follows is basically using bit wise masks to unpack the canId to print it out
      Serial.println("-----------------------------");
      Serial.println("Get data from ID: ");
      //Serial.println(canId, BIN);
      // just calling println with the BIN argument will almost do the right thing
      // but if there are leading zeros they get ignored
      // we want to print out the bits and then under them put the number of the bit
      // so we make a variable named mask with one bit one, compare the canId with the mask
      // and print a zero or one.  Then we shift the bit in mask that is on to the right one place
      for (unsigned long int mask = 0x80000000; mask; mask >>= 1) {
        Serial.print(mask & canId ? '1' : '0');
      }
      Serial.println();
      Serial.println("10987654321098765432109876543210");
      Serial.println(" 3         2         1");
      // next up do a hex dump of the data block
      for (int i = 0; i < len; i++) // print the data
      {
        Serial.print(buf[i], HEX);
        Serial.print("\t");
      }
      Serial.println();
      // Now FRC has specific packing of the canId.   we apply masks from FRC_CAN_utils.h to pick out the
      // different packed regions.  There are 5 parts in the canId that are NOT the data block:
      //  DEVICE type, Manufacturer, Class, Class Index, and Device Number
      //  You could have a type=code number for motor controller with manufacturer=code number for VexRobotics with
      //  class = code number for set parameter on controller with classIndex = code number for set speed on device
      //  number 3.
      // Of particular relevance there are codes for device=miscellaneous,  manufacturer = "team use".
      // the class and classIndex is defined for the device and manufacturer -- for example a Vex Motor controller
      // and a Jaguar motor controller could have completely different class and classIndex maps.
      // There is overlap with the defined device classes and some functionality of sensors you might hang
      // off an InCANceivable -- in case of overlap we take a myopic approach and route all the sensors through
      // miscellaneous, team use,  and will define different sensor data streams by class and classIndex.
      //
      // What follows will just do a dump of the data; we could also apply some logic here to see if there are
      // messages from a different InCANceivable on the same bus and dump it out with more interpretation.
      // But to keep things simple we just dump in a device agnostic way.
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
      Serial.println((canId >> FRC_DEVNUM_SHIFT) & FRC_DEVNUM_MASK , HEX);

      // As alluded to above, we will do something smarter as proof of concept
      if ((((canId >> FRC_CLASS_SHIFT)& FRC_CLASS_MASK)  == INCAN_CL_PIN) 
        && (((canId >> FRC_CLINDEX_SHIFT)& FRC_CLINDEX_MASK) == INCAN_PIN_DATA))
      {
        // arcane knowledge that the data block contains two ints, cast the data type
        unsigned int *d = (unsigned int*)buf;
        Serial.println("This is a message containg pin status data (4 bit masked integers)");
        Serial.println("First 16 bit integer contains bits");
        for (unsigned int mask = 0x8000; mask; mask >>= 1)
        {
          Serial.print(mask & d[0] ? '1' : '0');
        }
        Serial.println();
        Serial.println("5432109876543210");
        Serial.println("     1");
        /* Serial.println("High integer contains bits");
        for (unsigned int mask = 0x8000; mask; mask >>= 1)
        {
          Serial.print(mask & d[1] ? '1' : '0');
        }
        Serial.println();
        Serial.println("5432109876543210");
        Serial.println("     1");
        */
      }
    } // closes check to make sure this isn't first datum
  } // closes check for data

  if ((loopCount % 10000) == 0)
  {
    // we just want to have some sign of life on the serial bus if nothing is coming in

    Serial.print('.');
    if ((loopCount % 100000) == 0)
    {
      Serial.println(';');
    }

  }
}
