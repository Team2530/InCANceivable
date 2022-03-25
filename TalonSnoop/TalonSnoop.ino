#include <SPI.h>
#include <FRC_CAN_utils.h>
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>
#include <stdio.h>

#define TALON_MASK (4L << FRC_MANUFACT_SHIFT) | (2L << FRC_DEVICE_SHIFT)

// Seeed MCP2515 Hat:
const int CAN_INT_PIN = 2;
extern MCP_CAN CAN;

void setup() {
  Serial.begin(115200);
  Serial.println("Eeeeeeeeeeeee.");

  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING);

  Serial.println("Initializing CAN chip.");

  // Initialize the CAN chip
  while (CAN_OK != CAN.begin(FRC_CAN_SPEED)) delay(100);

  // Motor controller mask
  CAN.init_Mask(0, FRC_EXT, TALON_MASK);
  CAN.init_Filt(0, FRC_EXT, TALON_MASK);
  Serial.println("Init.");
}

void loop() {
  uint32_t canID;
  uint8_t CANbuf[8];
  uint8_t CANlen = 8;
  int messageAPIclass;
  int messageAPIindex;

//  // CAN mesg?
//  if (CANflagRecv) {
//    CANflagRecv = 0;
//    Serial.println("Message!");
  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&canID, &CANlen, CANbuf);
    Serial.print(canID);
    for (int i = 0; i < 8; ++i) {
      Serial.print(",");
      if (i < CANlen) Serial.print(CANbuf[i]);
      else Serial.print("0");
    }
    Serial.println();
    //InCANceivable_msg_dump(canID, CANlen, CANbuf);
  }
  //}
}
