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
unsigned char haveHeartbeat = 0;
int heartbeatPixel = 1;
unsigned char alliance = GREEN;
unsigned char enabled = 0;
unsigned char autonomous = 0;
unsigned char watchdog = 0;

unsigned char userStripProgramNumber = 2;
unsigned char chamberStatus[2] = { 0, 0 };
unsigned long canRefreshMillis = 1000L;
// we send a message out on the canBus every 50ms or when the status changes

unsigned long lastLedRefreshMillis = 0L;
unsigned long stripProgramRefreshMillis = 30L;

void setup() {
  // two lines to set up the CAN functionality
  myCanID = FRC_CAN_init();
  InCANceivable_setup();

  // set up light strip controller
  strip.begin();
  strip.setBrightness(64);  // if you crank the brightness up you pretty much just burn battery
  // visually it's not a lot different
  for (int i = 0; i < numLEDs; i++) {
    strip.setPixelColor(i, 0, 255, 0);
  }
  strip.show();
  // set up the I2C for sensors
  // probably want this here since there may be multiple
  // devices -- if each is in it's own library it may be
  // ugly to avoid name clashes
  Wire.begin();
  Wire.setClock(3400000);
#if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(100000, true);
#endif

  for (int i = 0; i < 2; ++i) {
    switchMux(i);
    // initColorSensor returns true on err
    if (initColorSensor()) {
      chamberStatus[i] = IDK;
    }
  }
  //  Serial.begin(115200);
  //  while (!Serial) {
  //    ;
  //  }
  //  Serial.println("start up done");
}

void loop() {
  static int canRunning = 0;
  unsigned long canID;
  unsigned char CANbuf[8];
  unsigned char CANlen = 8;
  int messageAPIclass;
  int messageAPIindex;
  unsigned long currentMillis;
  unsigned long loopBeginMillis;
  static unsigned long benchmarkTime = 0;
  static int loopCount = 0;
  int changed = 0;
  loopCount++;

  currentMillis = millis();
  //  if (loopCount==1000){
  //    loopCount=0;
  //     Serial.print("ave loop time (microsec):");
  //     Serial.println((currentMillis-benchmarkTime));
  //     benchmarkTime=currentMillis;
  //  }

  loopBeginMillis = currentMillis;
  //Serial.println(".");
  // check the can bus
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&canID, &CANlen, CANbuf);
    if (canRunning == 0) {
      canRunning = 1; // discard first CAN packet
    } else {
      //      
      if (FRC_CAN_isRIO(canID)) {
        // the RIO sends a universal heartbeat
        unsigned long heartbeatID = 0x01011840;
        //Serial.println("rio");

        if (canID == heartbeatID) {
          //Serial.println("beep");
          parseRioHeartbeatByte(CANbuf[4], currentMillis);
          lastHeartbeatMillis = currentMillis;
        } else {
          //InCANceivable_msg_dump(canID,CANlen,CANbuf);
        }
      } else {
        //Serial.println("calling isMe");
        if (FRC_CAN_isMe(canID, myCanID)) {
          int apiClass;
          int apiIndex;
          FRC_CAN_extractClass(canID, &apiClass, &apiIndex);
          //Serial.println(apiClass);
          switch (apiClass) {
            case INCAN_CL_SLED:
              userStripProgramNumber = CANbuf[0];
              // n.b. we will actually call the function later
              // running the user strip program isn't taken to have
              // high priority -- we run them periodically and then only
              // after we have read sensors and made sure there's not a fresh
              // can message waiting for us on the next loop pass
              break;
          }
        } else {
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
  //Serial.println(changed);
  if (changed) {
    // tell the canbus about it and update the LED's
    sendChamberStatus(chamberStatus, myCanID);
    updateChamberStatusLEDs(chamberStatus);
    lastStatusSendMillis = currentMillis;
  } else {
    // hasn't changed, if it's been a long time then re-tell the canbus
    // this may be overkill but its relatively cheap insurance in case
    // we are running long before roborio starts or is rebooting etc
    if ((currentMillis - lastStatusSendMillis) >= canRefreshMillis) {
      sendChamberStatus(chamberStatus, myCanID);
      lastStatusSendMillis = currentMillis;
    }
  }
  // finally we do the fluff task of updating the LEDs for user programs
  // we only want to do that if there is NOT a fresh CAN message waiting for us
  // it's possible, for example,  that we got a fresh message while we were reading sensors
  // and setting feedback LEDs -- processing that is more important animation or running
  // user LED programs
  if (CAN_MSGAVAIL != CAN.checkReceive()) {
    currentMillis = millis(); // we may have unpredictable timing (could be appreciable, so freshen the clock)
    if ((currentMillis - lastLedRefreshMillis) >= stripProgramRefreshMillis) {
      // we rely on the stripProgramX's to remember state and keep track of timing so all we have to do is
      // tell them the current clock
      //Serial.println(userStripProgramNumber);
      switch (userStripProgramNumber) {
        case 0:
          stripProgram0(currentMillis);
          break;
        case 1:
          stripProgram1(currentMillis);
          break;
        case 2:
          stripProgram2(currentMillis);
          break;
        default:
          stripProgram0(currentMillis);
          break;
      }
    }
  }

  // just to help us understand our deadtime -- how variable is it?
  currentMillis = millis();

  // Something took too long to run, maybe one of the I2C devices isn't connected.
  // So we go through all of the sensors and make sure they are there.
  // Although, I2C errors should already be handled...
  // But no matter what, we don't want the board to die from timeout because of something hanging.
  if (currentMillis - loopBeginMillis > 200) {
    // Prevent imminent death
    lastHeartbeatMillis = millis();

    for (int i = 0; i < 2; ++i) {
      switchMux(i);
      Wire.beginTransmission(COLORSENSORV3_ADDR);
      byte error = Wire.endTransmission();
      if (error != 0) {
        chamberStatus[i] = IDK;
      }
    }
  }

  // Serial.print('loop time:');
  // Serial.println(currentMillis - loopBeginMillis); // we can use the plotter to peak at this

  // We've processed CAN -- if currentMillis is more than 100ms newer than lastHeartbeatMillis we have missed the
  // heartbeats and per FRC should assume that rio is dead.
  // if ((currentMillis -  // We've processed CAN -- if currentMillis is more than 100ms newer than lastHeartbeatMillis we have missed the
  // heartbeats and per FRC should assume that rio is dead.
  if ((currentMillis - lastHeartbeatMillis) > 100L) {
    haveHeartbeat = 0;
    strip.setPixelColor(heartbeatPixel, 139, 0, 0); // dark red
    strip.show();  // we are not expecting to call this often so no harm in showing
    // if we are hitting it hard things are broken anyway
  }
}


// some example LED strip programs
void stripProgram0(unsigned long now) {
  // turns pixels off
  int userLow = 35;
  int userHigh = 50;
  for (int i = userLow; i < userHigh; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}

void stripProgram1(unsigned long now) {
  // implements a toggling pair of patterns r/g and g/r
  static unsigned long lastStart = 0;
  unsigned long patternTimes[2] = { 250, 1000 };
  static int currentPattern = 2; // initialize out of bounds
  int newPattern;
  int r;
  int g;
  int b;
  int userHigh = 50;
  int userLow = 35;
  unsigned long delta;
  delta = now - lastStart;
  if (delta > patternTimes[1]) {
    lastStart = now;
    delta = 0;
  }
  newPattern = 1;
  for (int i = 0; i < 2; i++) {
    if (delta > patternTimes[i]) {
      newPattern = i;
    }
  }
  //  Serial.print("delta:");
  //  Serial.print(delta);
  // Serial.print(" ");
  //  Serial.println(newPattern);
  if (newPattern != currentPattern) {
    currentPattern = newPattern;
    //Serial.println(currentPattern);
    switch (currentPattern) {
      case 0:
        //      Serial.println(0);
        r = 0; g = 255; b = 0;
        for (int i = userLow; i < userHigh; i += 2) {
          strip.setPixelColor(i, r, g, b);
        }
        r = 255; g = 0; b = 255;
        for (int i = userLow + 1; i < userHigh; i += 2) {
          strip.setPixelColor(i, r, g, b);
        }
        break;
      case 1:
        //        Serial.println(1);
        r = 255; g = 0; b = 255;
        for (int i = userLow; i < userHigh; i += 2) {
          strip.setPixelColor(i, r, g, b);
        }
        r = 0; g = 255; b = 0;
        for (int i = userLow + +1; i < userHigh; i += 2) {
          strip.setPixelColor(i, r, g, b);
        }
        break;
    }
    strip.show();
  }
  //  else {
  //     same pattern as before don't change a thing
  //  }

}

void stripProgram2(unsigned long now) {
  // for fun we'll make tri colored snakes rolling around the leds
  int highLED = 70;
  int lowLED = 35;
  int snakeRed[15] = { 0,20,60,120,255,   120,60,20,0,0,  0,0,0,0,0 };
  int snakeGreen[15] = { 0,0,0,0,20, 60,120,255,120,60, 20,0,0,0,0 };
  int snakeBlue[15] = { 0,0,0,0,0, 0,0,20,60,120, 255,120,60,20,0 };
  static unsigned long lastUpdate = 0;
  unsigned long updateTime = 75L;
  static int offset = 0;

  if ((now - lastUpdate) > updateTime) {
    //Serial.println("prog 2");
    lastUpdate = now;
    for (int i = 0;i < (highLED - lowLED);i++) {
      int colorIndex = (i + offset) % 15; // mod the length of the snake<color> arrays
      strip.setPixelColor(i + lowLED, snakeRed[colorIndex], snakeGreen[colorIndex], snakeBlue[colorIndex]);
    }
    // in theory we could just let offset run on forever -- but it's a 16bit int and will eventually roll over
    // clamp it now
    offset++;
    offset = offset % (highLED - lowLED);  // mode the length of the section leds
  }
  strip.show();
}


// utility functions defined below
// per https://docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html
void parseRioHeartbeatByte(unsigned char inByte, unsigned long currentMillis) {
  static unsigned long lastMillis = 0;
  unsigned long totalCycle = 500;
  unsigned long off = 250;
  int alliancePixel = 0;
  int enabledPixel = 1;
  int autonomousPixel = 2;
  int watchDogPixel = 3;
  if (~haveHeartbeat) {
    strip.setPixelColor(heartbeatPixel, 50, 255, 50);
    haveHeartbeat = 1;
  }
  //Serial.println('.');
  if (inByte & 0x1) {
    alliance = RED;
    strip.setPixelColor(alliancePixel, 255, 0, 0);
  } else {
    alliance = BLUE;
    strip.setPixelColor(alliancePixel, 0, 0, 255);
  }
  if (inByte & 0x1) {
    enabled = 1;
    if ((currentMillis - lastMillis) < off)
      strip.setPixelColor(enabledPixel, 0, 0, 0);
    else
      strip.setPixelColor(enabledPixel, 255, 120, 0);
    if ((currentMillis - lastMillis) > totalCycle) lastMillis = currentMillis;
  } else {
    strip.setPixelColor(enabledPixel, 64, 20, 0); //1/4 brightness
    enabled = 0;
  }
  if (inByte & 0x4) {
    autonomous = 1;
    strip.setPixelColor(autonomousPixel, 255, 120, 0);
  } else {
    autonomous = 0;
    strip.setPixelColor(autonomousPixel, 255, 0, 255);
  }
  if (inByte & 0x10) {
    watchdog = 1;
    strip.setPixelColor(watchDogPixel, 0, 255, 0);
  } else {
    watchdog = 0;
    strip.setPixelColor(watchDogPixel, 255, 0, 0);
  }
  strip.show();
}

void sendChamberStatus(unsigned char* statusbytes, unsigned long canID) {
  int APIClassId = INCAN_CL_CHUTE;
  int APIIndex = 0;
  int len = 2;
  canID = FRC_CAN_embed(canID, APIClassId, APIIndex);
  CAN.sendMsgBuf(canID, FRC_EXT, 2, statusbytes);
}

//int detectBalls(unsigned char *statusbytes) {
// read the sensors on I2c compare logical output to statusbytes
// if different then update statusbytes and return 1
// if no changes return 0;
//}

void updateChamberStatusLEDs(unsigned char* buf) {
  int chamber0[2] = { 10, 19 };
  int chamber1[2] = { 20, 29 };
  int rgb[3] = { 0, 0, 0 };
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
