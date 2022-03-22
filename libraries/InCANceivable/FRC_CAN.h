#ifndef FRC_CAN_INCLUDED
#define FRC_CAN_INCLUDED
/*
FRC_CAN.h

This is a landing spot for Defines that should be good to let a Seeed Studio CANBed board play nicely
on the CAN bus for an FRC robot.  We assume you have the CANBed libary in place and will pull in the
.h files from there so we can use symbols defined there in our definitions.

*/

#include <mcp_can_dfs.h>

/* First off, we define a bunch of symbols that make it more transparent when we build up FRC specific
   "arbitration ID's"  */

#define FRC_EXT  1  
   // always extended 29 bit addresses for FRC 

#define FRC_CAN_SPEED CAN_1000KBPS  
// always run at 1MBS;  this is inherited from the mcp_can_dfs.h  

#define FRC_DEVICE_SHIFT 24 
#define FRC_MANUFACT_SHIFT 16
#define FRC_CLASS_SHIFT 10
#define FRC_CLINDEX_SHIFT 6
#define FRC_DEVNUM_SHIFT 0
// Now you can take construct your Arbitration ID 
// ARB = (devL<<FRC_DEVICE_SHIFT) | (manufL<<FRC_MANUFACT_SHIFT) | (apiClassL<<FRC_CLASS_SHIFT) | (apiIndexL<<FRC_CLINDEX_SHIFT) | devNumL
// where L indicates that the variable is a an unsigned long int
// 
// Then you would like to make it straight forward to deconstruct an Arbitration ID
#define FRC_DEVICE_MASK 0x1fL 
#define FRC_MANUFACT_MASK 0xffL
#define FRC_CLASS_MASK 0x3fL
#define FRC_CLINDEX_MASK 0xfL
#define FRC_DEVNUM_MASK 0x3fL
// this allows constructs like  Manuf=(ARB>>FRC_MANUFACT_SHIFT) & FRC_MANUFACT_MASK to extract fields. 

/* Now we define some things that are specific to our software stack */

#define INCAN_DEVICE_TYPE 10L
// miscellaneous device
#define INCAN_MANUFACT  8L
// team use manufacturer 


#define INCAN_MASK   (INCAN_DEVICE_TYPE<<FRC_DEVICE_SHIFT) | (INCAN_MANUFACT<<FRC_MANUFACT_SHIFT)
// every message *we* will send will have and Arbitation ID built by adding to INCAN_MASK

#define RIO_MASK (1L<<FRC_DEVICE_SHIFT) | (1L<<FRC_MANUFACT_SHIFT)
/// aka device is robot controller and manufacturer is NI  -- we expect to care about messages from the RIO so go ahead and define it


// One notable case: we will actually generate messages that don't begin with INCAN_MASK in the FRC_acceptance.ino sketch 
// In that case we will want to fake sending messages from the RIO and the all-zeros emergency stop mask

//  We may want to mask so we only accept messages from all zeros (broadcast) 
//  or RIO_MASK for control or another InCANceivable (for debugging). 

// FRC does some kind of weird stuff by highjacking part of the DEVICE id to specify 
// protocol and operations.  Naively one might have expected the device ID to reflect the 
// physical hardware and then use the "data" segment to transfer commands etc. But this approach
// saves some bits on the wire. We previously defined symbols that help with the bit packing and unpacking 
// Below are the API class and index definitions for specific to this package. 
// This becomes quite arbitrary and is an artifact of the team hardware specific implementation. 

// API CLASSES  (6 bits) 
#define INCAN_CL_CFG 1L  // we could use zero but avoid it as an extra layer of defense 
// against stepping on broadcast messages with class num zero
#define INCAN_CL_PIN 5L  // board pins
#define INCAN_CL_ACC 10L // I2c accelerometer
#define INCAN_CL_PRX 15L  // I2c proximity sensor
#define INCAN_CL_ANAUS 16L  // Analog Ultrasound/ 
#define INCAN_CL_GPIO 20L // I2c 4 pin GPIO 
#define INCAN_CL_SLED 25L // Addressable Strip LED
#define INCAN_CL_CHUTE 32L // 
// not all of the defines below are supported -- this is lifted from the can-addressing.html 
// page from wpilip,org 
#define FRC_BROADCAST_DISABLE 0
#define FRC_BROADCAST_HALT 1
#define FRC_BROADCAST_RESET 2
#define FRC_BROADCAST_DEVASSIGN 3
#define FRC_BROADCAST_DEVQUERY 4
#define FRC_BROADCAST_HEARTBEAT 5
#define FRC_BROADCAST_SYNC 6
#define FRC_BROADCAST_UPDATE 7
#define FRC_BROADCAST_FIRMWARE 8
#define FRC_BROADCAST_ENUMERATE 9
#define FRC_BROADCAST_RESUME 10

// Each Class has set of functions (determined by an "index value" (as in FRC_CLINDEX_*)
// API INDEX DEFINES (4 bits) -- we will set up three suggested indices that may be useful across 
// all API classes -- disable, enable and "this is data message"
#define INCAN_ALL_DIS 0
#define INCAN_ALL_EN 1 
#define INCAN_ALL_DATA 8 
// This would let someone quickly filter for data 
#define INCAN_DATA_MASK = INCAN_MASK | (INCAN_ALL_DATA << INCAN_CLINDEX_SHIFT)
// So -- conceptuall one could filter  if (ARB & INCAN_DATA_MASK ) == INCAN_DATA_MASK 
//   then you care about investigating further -- otherwise it's not data from the INCAN.


// Now we get down into nasty class specific indices that you may not really care about 
// most of the time. 
#define INCAN_CFG_DIS 0
#define INCAN_CFG_EN 1
// OVERALL INCAN_CFG DIS and EN  refer to whether or not we 
// generate messages on the bus automatically.  If you want the INCAN 
// to free run and periodically dump data on the CAN-bus then EN
// otherwise disable and send INCAN_CFG_SS (single shot) causing single
// dumps of one reading from all the sensors etc to be performed

#define INCAN_CFG_SS 2
#define INCAN_CFG_DEL 3
// DEL== delay (ms)
#define INCAN_CFG_ERR 4

#define INCAN_PIN_DIS 0
#define INCAN_PIN_EN 1
#define INCAN_PIN_SET 2
// Data is pin number and state 
#define INCAN_PIN_PWM -1
#define INCAN_PIN_ADC -1
#define INCAN_PIN_DATA 8
// INCAN_PIN_DATA is binary read of the pins
// N.B. PWM and ADC unsupported; assign positive numbers when supported

#define INCAN_ACC_DIS 0L 
#define INCAN_ACC_EN 1L 
#define INCAN_ACC_SMTH -2L
// data value is number of read values to smooth over
#define INCAN_ACC_BP -3L
// data value is high and low freqs for bandpass filter
#define INCAN_ACC_SENDXYZ -4L  
// data is 0 or 1 for do or do not (default 0) 
#define INCAN_ACC_SENDGRADS -5L  
// data is 0 or 1 for do or do not (default 1) 
#define INCAN_ACC_DATA 8L 
// data will be packed as follows:
// int what = int* buf[4-7]
// what is the direction 1,2,3 for X,Y,Z
// if what = 4 then this is the angle of the xy plane with respect to 
// gravity 
// if what = 5 this same as in 4 but in gradians (slope )
// float value = float*  buf[0-3]

#define INCAN_PRX_DIS 0
#define INCAN_PRX_EN 1
#define INCAN_PRX_CAL 2 
// CAL is a call to use the last few measurment of the sensor tod 
// define a threshold for "is different"  -- the idea is 
// that the proximity sensor is likely being used to detect 
// the presence or absence of a game piece -- so this would be 
// measuring w/o a piece in place.  Data for CAL may eventually 
// be some sort of threshold parameter (likelihood of false negative ?)   
// 
#define INCAN_PRX_DATA 8 


#define INCAN_ANAUS_DIS 0
#define INCAN_ANAUS_EN 1
#define INCAN_ANAUS_DATA 8  // data as 2 floats; 
//  flt[0]=average; flt[1]=most recent reading. 


#define INCAN_GPIO_DIS 0
#define INCAN_GPIO_EN 1
#define INCAN_GPIO_SET 2
// Data is pin numbers and desired states (bit-wise)  
#define INCAN_GPIO_DATA 8


#define INCAN_SLED_DIS 0
#define INCAN_SLED_EN 1
#define INCAN_SLED_SOLID 2
// DATA IS RGB byte triplet
#define INCAN_SLED_PROG 3
// DATA is program number 
// no INCAN_SLED_DATA since it's output only

#endif
