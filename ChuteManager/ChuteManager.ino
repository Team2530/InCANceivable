#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>
#include <ColorSensor.h>
#include <Wire.h>

#define IDK 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define NOBALL 4

#define PIN_STRIP1 11
const int numLEDs = 80;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLEDs, PIN_STRIP1, NEO_GRB + NEO_KHZ800);


extern MCP_CAN CAN;
unsigned long myCanID;
unsigned long lastStatusSendMillis = 0;

// stuff we track coming over from the rio
unsigned long lastHeartbeatMillis = 0;
unsigned char alliance = GREEN;
unsigned char enabled = 0;
unsigned char autonomous = 0;
unsigned char watchdog = 0;

unsigned char userStripProgramNumber = 0;
unsigned char chamberStatus[2] = {0, 0};
unsigned long canRefreshMillis = 1000;
// we send a message out on the canBus every 50ms or when the status changes

unsigned long lastLedRefreshMillis = 0;
unsigned long ledRefreshMillis = 10;

void setup() {

  // two lines to set up the CAN functionality
  myCanID = FRC_CAN_init();
  InCANceivable_setup();

  // set up light strip controller
  strip.begin();
  strip.setBrightness(64);  // if you crank the brightness up you pretty much just burn battery
  // visually it's not a lot different
  for (int i = 0; i < numLEDs; i++)
  {
    strip.setPixelColor(i, 0, 255, 0);
  }
  strip.show();
  // set up the I2C for sensors
  // probably want this here since there may be multiple
  // devices -- if each is in it's own library it may be
  // ugly to avoid name clashes
  Wire.begin();
  Wire.setClock(3400000);
  switchMux(0);
  initColorSensor();
  switchMux(1);
  initColorSensor();
  //Serial.begin(115200);
  //while (!Serial) {
  //  ;
  //}

}

void loop() {
  static int canRunning = 0;
  unsigned long canID;
  unsigned char CANbuf[8];
  unsigned char CANlen = 8;
  int messageAPIclass;
  int messageAPIindex;
  unsigned long currentMillis;
  int changed = 0;
  currentMillis = millis();
  //Serial.println(".");
  // check the can bus
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&canID, &CANlen, CANbuf);
    if (canRunning == 0) {
      canRunning = 1; // discard first CAN packet
    }
    else {
      if (FRC_CAN_isRIO(canID)) {
        // the RIO sends a universal heartbeat
        unsigned long heartbeatID = 0x01011840;
        if (canID == heartbeatID) {
          parseRioHeartbeatByte(CANbuf[4]);
          lastHeartbeatMillis = currentMillis;
        }
      }
      else {
        if (FRC_CAN_isMe(canID, myCanID)) {
          int apiClass;
          int apiIndex;
          FRC_CAN_extractClass(canID, &apiClass, &apiIndex);
          //          switch (apiClass) {
          //            case XXX:
          //              stuff
          //              break;
          //            case setprogram:
          //              userStripProgramNumber = buf[0];
          //              break;
          //            default:
          //              // if we wanted to complain about bad classes ..
          //              break;
          //          }
        }
        else {
          if (FRC_CAN_isBroadcast(canID)) {
            // empirically this is a rare message
            if (canID == 0) FRC_CRASH(0);
          }
        }
      }
    }
  }
  //Serial.println(millis()-currentMillis);
  // read sensors and send message to rio if chamberStatus changed
  // OR if it's been a while
  changed = detectBalls(chamberStatus, 2);
  //Serial.println(millis()-currentMillis);
  //Serial.println(changed);
  if (changed) {
    // update the LED's
    //Serialprint(chamberStatus[0], HEX);
    //Serialprintln(chamberStatus[1], HEX);
    sendChamberStatus(chamberStatus, myCanID);
    updateChamberStatusLEDs(chamberStatus);
    lastStatusSendMillis = currentMillis;
  }
  else {
    if ((currentMillis - lastStatusSendMillis) >= canRefreshMillis) {
      sendChamberStatus(chamberStatus, myCanID);
      lastStatusSendMillis = currentMillis;
    }
  }
  // finally we do the fluff task of updating the LEDs for user programs
  // we only want to do that if there is NOT a can message waiting for us
  // it's possible, for example,  that we got a fresh message while we were reading sensors
  // and setting feedback LEDs -- processing that is more important animation
  if (CAN_MSGAVAIL != CAN.checkReceive()) {
    if ((currentMillis - lastLedRefreshMillis) >= ledRefreshMillis) {

      switch (userStripProgramNumber) {
        case 1:
          stripProgram1(currentMillis);
          break;

        default:
          break;
      }
    }
  }
}
void stripProgram0(unsigned long now) {
  int userLow = 35;
  int userHigh = 50;
  for (int i = userLow; i < userHigh; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
}

void stripProgram1(unsigned long now) {
  static unsigned long lastStart = 0;
  unsigned long change = 250;
  int state = 0;
  int r;
  int g;
  int b;
  int userHigh = 50;
  int userLow = 35;
  unsigned long delta;
  delta = now - lastStart;
  if (delta > change) {
    lastStart = now;
    state++;
    state = state % 2;
  }

  if (state) {
    r = 0; g = 255; b = 0;
    for (int i = userLow; i < userHigh; i += 2) {
      strip.setPixelColor(i, r, g, b);
    }
    r = 255; g = 0; b = 255;
    for (int i = userLow + 1; i < userHigh; i += 2) {
      strip.setPixelColor(i, r, g, b);
    }
  }
  else {
    r = 0; g = 255; b = 0;
    for (int i = userLow + 1; i < userHigh; i += 2) {
      strip.setPixelColor(i, r, g, b);
    }
    r = 255; g = 0; b = 255;
    for (int i = userLow; i < userHigh; i += 2) {
      strip.setPixelColor(i, r, g, b);
    }
  }
}

// utility functions defined below
// per https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html
void parseRioHeartbeatByte(unsigned char inByte) {
  if (inByte & 0x1) alliance = RED;
  else alliance = BLUE;
  if (inByte & 0x2) enabled = 1;
  else enabled = 0;
  if (inByte & 0x4) autonomous = 1;
  else autonomous = 0;
  if (inByte & 0x10) enabled = 1;
  else enabled = 0;
}

void sendChamberStatus(unsigned char *statusbytes, unsigned long canID) {
  int APIClassId = INCAN_CL_CHUTE;
  int APIIndex = 0;
  int len = 2;
  canID = FRC_CAN_embed(canID, APIClassId, APIIndex);
  CAN.sendMsgBuf(canID, FRC_EXT, 2, statusbytes);
}

int detectBalls(unsigned char *statusbytes) {
  // read the sensors on I2c compare logical output to statusbytes
  // if different then update statusbytes and return 1
  // if no changes return 0;
}

void updateChamberStatusLEDs(unsigned char *buf) {
  int chamber0[2] = {0, 10};
  int chamber1[2] = {20, 30};
  int rgb[3] = {0, 0, 0};
  switch (buf[0]) {
    case IDK:
      rgb[0] = 255; rgb[1] = 165; rgb[2] = 0;
      break;
    case RED:
      rgb[0] = 255; rgb[1] = 0; rgb[2] = 0;
      break;
    case BLUE:
      rgb[0] = 0; rgb[1] = 0; rgb[2] = 255;
      break;
    default:
    case NOBALL:
    case GREEN:
      rgb[0] = 0; rgb[1] = 255; rgb[2] = 0;
      break;
  }
  for (int i = chamber0[0]; i < chamber0[1]; i++) {
    strip.setPixelColor(i, rgb[0], rgb[1], rgb[2]);
  }

  switch (buf[1]) {
    case IDK:
      rgb[0] = 255; rgb[1] = 165; rgb[2] = 0;
      break;
    case RED:
      rgb[0] = 255; rgb[1] = 0; rgb[2] = 0;
      break;
    case BLUE:
      rgb[0] = 0; rgb[1] = 0; rgb[2] = 255;
      break;
    default:
    case NOBALL:
    case GREEN:
      rgb[0] = 0; rgb[1] = 255; rgb[2] = 0;
      break;


  }
  for (int i = chamber1[0]; i < chamber1[1]; i++) {
    strip.setPixelColor(i, rgb[0], rgb[1], rgb[2]);
  }
  strip.show();
}
