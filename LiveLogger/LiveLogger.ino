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

  //attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING);

  // Initialize the CAN chip
  while (CAN_OK != CAN.begin(FRC_CAN_SPEED)) delay(100);

  // No masks/filters.
  CAN.init_Mask(0, FRC_EXT, 0);
  CAN.init_Filt(0, FRC_EXT, 0);
}

void loop() {
  uint32_t canID;
  uint8_t CANbuf[8];
  uint8_t CANlen = 8;

  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&canID, &CANlen, CANbuf);
    Serial.print(millis());
    Serial.print(",");
    Serial.print(canID);
    for (int i = 0; i < 8; ++i) {
      Serial.print(",");
      if (i < CANlen) Serial.print(CANbuf[i]);
      else Serial.print("0");
    }
    Serial.println();
  }
}
