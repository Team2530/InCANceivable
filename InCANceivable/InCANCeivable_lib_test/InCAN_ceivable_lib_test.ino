#include <InCANceivable_globals.h>
#include <InCANceivable.h>

unsigned long int lastDumpMillis=300;  
// Initialize non-zero so we can gather some sensor 
// readings before starting to send data. 
unsigned long int millisBetweenDumps=200; 
// wait at least this long between outbound data dumps
// first dump will happen lastDumpMillis+millisBetweenDumps after the end of the setup() function (~500ms)

void setup() {
  // put your setup code here, to run once:
 InCANceivable_setup();
 lastDumpMillis+=millis(); // this is just syncing up the clock before we dive into loop
}

void loop() 
{
  // put your main code here, to run repeatedly:
 unsigned long int canId;
 unsigned long int now;
 while (messageCheck(&canId)) 
  {
    // handle incoming messages (probably from RIO) here
    Serial.println("message to check");
    // control message parsing goes here.... 
    InCANceivable_msg_dump(canId);
  }

  // . here you would put code to update your sensor values

  
  // if enough time has passed send sensor data on the bus
  now=millis();       
  if (now > (lastDumpMillis+millisBetweenDumps))
  {
    lastDumpMillis=now;
    //   
    // putting data from sensors on bus goes here
    // 
  }
 // if you are using LED's to indicate heartbeats then set the LED pin
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,FRC_CAN_heartBeat(0));
}
