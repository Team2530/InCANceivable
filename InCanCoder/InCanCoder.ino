#include <AS5145.h>

#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>
#include <SPI.h>

#define ENC_CS A0
#define ENC_DO A2
#define ENC_CLK A1
#define ENC_PROG 6

// 100ms is problably OK
#define BROADCAST_INTERVAL 100

extern MCP_CAN CAN;
AS5145 enc(ENC_DO, ENC_CLK, ENC_CS, ENC_PROG);

uint32_t myCanID;

void setup() {
  // Set up InCANceivable & PHY
  myCanID = FRC_CAN_init();
  InCANceivable_setup();
}

uint16_t enc_value;
uint32_t last_broadcast;

void loop() {
  enc_value = enc.encoder_value();
  
  if ((millis() - last_broadcast) > BROADCAST_INTERVAL) {
    send_encoders(myCanID);
    last_broadcast = millis();
  }
}

struct data {
  uint16_t value;
};

void send_encoders(uint32_t canID) {
  int APIClassId = INCAN_CL_ENCODER;
  int APIIndex = 0;
  canID = FRC_CAN_embed(canID, APIClassId, APIIndex);

  struct data dat;
  dat.value = enc_value;
  
  CAN.sendMsgBuf(canID, FRC_EXT, sizeof(struct data), (uint8_t*)&dat);
}
