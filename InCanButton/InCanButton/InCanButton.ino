
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
extern MCP_CAN CAN; 
unsigned long int myCanId;
byte buf[8];
int len=8;
#define BUTTONPIN 4
int oldButtonState=0;
void setup() {
   Serial.begin(115200);
   while (!Serial) {
   ;
    }
  // put your setup code here, to run once:
 pinMode(BUTTONPIN, INPUT);
  myCanId=FRC_CAN_init();  // sets speed, extended protocol etc
  InCANceivable_setup();
}

void loop() {
  
 unsigned long int outputCanId= FRC_CAN_embed(myCanId, 1, 9);
  // put your main code here, to run repeatedly:
 int  buttonState;
 buttonState = digitalRead(BUTTONPIN);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState!=oldButtonState){
    buf[0]=buf[0]+1;
    CAN.sendMsgBuf(outputCanId,FRC_EXT,3, buf);
// CAN.sendMsgBuf(outputCanId,FRC_EXT,1,8, buf);
    oldButtonState=buttonState;
    Serial.println(".");
    //Serial.println(outputCanId,HEX);
    InCANceivable_msg_dump(outputCanId,8,buf);
   //  InCANceivable_msg_dump(myCanId,0,);
  }
}
