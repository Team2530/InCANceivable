#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>
#include <ColorSensor.h>
#include <SparkFun_VCNL4040_Arduino_Library.h>
#include <Wire.h>
#include <matrix.h>
#define IDK 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define NOBALL 4

#define PROXPIN0 6
#define PROXPIN1 5
#define PROX_NEAR 10
#define PROX_NOT_NEAR 8

#define PIN_STRIP1 11
const int numLEDs = 72;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLEDs, PIN_STRIP1, NEO_GRB + NEO_KHZ800);
#define MAX_STRIP_BRIGHTNESS 64
#define NUM_REV_LIGHT_SENSORS 4

  VCNL4040 sensor0;
  VCNL4040 sensor1;
//  proxSensors[0] = &sensor0;
//  proxSensors[1] = &sensor1;
//  proxSensors[2] = NULL;
//  proxSensors[3] = NULL;
VCNL4040* proxSensors[4]={&sensor0, &sensor1, NULL, NULL};  // we will use the convention that if proxSensors[i]==NULL, we don't use it
int proxPins[4]={PROXPIN0,PROXPIN1,-1, -1};  // and if pin=-1 we won't use that either 
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

unsigned char userStripProgramNumber = 0;  // if userStripProgramNumber>0 we run that program ;
// program 0 is display ball detection
unsigned char chamberStatus[8] = {NOBALL, NOBALL, NOBALL, NOBALL, NOBALL, NOBALL, NOBALL, NOBALL};
unsigned long canRefreshMillis = 1000L;
// we send a message out on the canBus every 1000ms or when the status changes

unsigned long lastLedRefreshMillis = 0L;
unsigned long stripProgramRefreshMillis = 30L;
int stripBrightness = 64;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  // two lines to set up
  myCanID = FRC_CAN_init();
  InCANceivable_setup();
  // set up light strip controller -- we do this first because we use it to monitor the boot sequence.
  strip.begin();
  strip.setBrightness(stripBrightness);  // if you crank the brightness up you pretty much just burn battery
  // visually it's not a lot different
  for (int i = 0; i < numLEDs; i++) {
    strip.setPixelColor(i, 255, 128, 0);
    // flood fill all yellow
  }

  for (int col = 0; col < 9 ; col++) {
    RGB color;
    int row = 0;
    color.r = 16; color.g = 128; color.b = 0;
    matrixPutPixel(&strip, color, col, row, 9, 8); // top row indicates CAN setup
    //    Serial.println(col);
  }
  strip.show();
  // Serial.println("done setting up CAN");
  // set up the I2C for sensors
  // probably want this here since there may be multiple
  // devices -- if each is in it's own library it may be
  // ugly to avoid name clashes
  Wire.begin();
  Wire.setClock(400000);
#if defined(WIRE_HAS_TIMEOUT)
  //Serial.println("setting WireTimeout");
  Wire.setWireTimeout(1000, true);
#endif

  for (int i = 0; i < NUM_REV_LIGHT_SENSORS; ++i) {
   // Serial.print("initializing sensor " );
   // Serial.println(i);
    //switchMux(i);
    // initColorSensor returns true on err
    if (initColorSensor()) {
      chamberStatus[i] = NOBALL;  // we could set this as IDK and then we'll never try to read it again;
      // setting it to NOBALL so we give it one shot in the main ballDetection loop before giving up.
      // if we end up hanging we can change this
      for (int col = 0; col < 9 ; col++) {
        RGB color;
        int row = i + 1;
        color.r = 256; color.g = 5; color.b = 0;
        matrixPutPixel(&strip, color, col, row, 9, 8);  // oops color sensor bad
      }
      strip.show();
    }

    else {
      // initialization ok,  calibrate the distance
      calibrateBallDetection(i);
      for (int col = 0; col < 9 ; col++) {
        RGB color;
        int row = i + 1;
        color.r = 16; color.g = 128; color.b = 128;
        matrixPutPixel(&strip, color, col, row, 9, 8);  //color sensor ok.
      }
      strip.show();
    }
  }
  //Serial.println("trying to initialize proximity sensors");
 
  for (int i = 0; i < 2; i++) {
    unsigned long currentMillis;
    VCNL4040 *sensor = NULL;
    switchMux(i + 4); // proximity is on ports 4,5
    delay(10);
    currentMillis = millis();
    sensor = proxSensors[i];
    if (sensor) {
      sensor->begin(Wire); // n.b. you have to pass Wire or the
      // VCNL4040 library will start it's own and then things
      // get really messy
      if (sensor->isConnected()) {
        sensor->powerOnProximity();
        sensor->powerOffAmbient();
        sensor->disableWhiteChannel();
        sensor->setProxIntegrationTime(1); // can be up to 8 
        sensor->setProxHighThreshold(PROX_NEAR); // magic empirical number
        sensor->setProxLowThreshold(PROX_NOT_NEAR); // another magic empirical number
        sensor->setProxInterruptType(VCNL4040_PS_INT_CLOSE); // PIN goes low when  balld is near
        sensor->enableProxLogicMode();
        pinMode(proxPins[i],INPUT);
        for (int col = 0; col < 9 ; col++) {
          RGB color;
          int row = i + 1 + 4;
          color.r = 16; color.g = 128; color.b = 0;
          matrixPutPixel(&strip, color, col, row, 9, 8);  //VCNL4040 sensor ok.
        }
        strip.show();
      }
    }
    else {
      //  Serial.print("failed to get to connect sensor ");
      //  Serial.print(i);
      //   Serial.println(" disabling");
      for (int col = 0; col < 9 ; col++) {
        RGB color;
        int row = i + 1 + 4;
        color.r = 256; color.g = 5; color.b = 0;
        matrixPutPixel(&strip, color, col, row, 9, 8);  //VCNL4040 sensor not ok.
      }
      strip.show();

      proxSensors[i] = NULL;
      strip.show();
      delay(2000);
    }
  }
  //
  updateChamberStatusLEDs(chamberStatus, 4);
  // Serial.println("start up done");
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
  //Serial.println(loopCount);
  currentMillis = millis();
  loopBeginMillis = currentMillis;

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    //    Serial.println(".");
    CAN.readMsgBufID(&canID, &CANlen, CANbuf);
    if (canRunning == 0) {
      canRunning = 1; // discard first CAN packet
    } else {
      if (FRC_CAN_isRIO(canID)) {
        // the RIO sends a universal heartbeat
        unsigned long heartbeatID = 0x01011840;

        if (canID == heartbeatID) {
          //Serial.println("beep");
          parseRioHeartbeatByte(CANbuf[4], currentMillis);
          lastHeartbeatMillis = currentMillis;
        }
        //else {  it was a message with RIO's canID but not a heartBeatID -- would handle that here
        //InCANceivable_msg_dump(canID,CANlen,CANbuf);
        //}
      } else {
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
            // empirically this is a rare message -- e.g. we've never seen it in practice.
            if (canID == 0) FRC_CRASH(0);
          }
        }
      }
    }
  }
  //
  // done checking the CAN bus
  // read sensors and send message to rio if chamberStatus changed
  // OR if it's been a while
  //Serial.println("calling detectBalls_prox");
  changed = detectBalls_prox_interrupt(chamberStatus, 4, proxPins);
  if ((loopCount%100)==0 ){
    Serial.println(millis()-currentMillis);
  }
  //Serial.println(changed);
  if (changed) {
    // tell the canbus about it and update the LED's
    sendChamberStatus(chamberStatus, 4, myCanID);
    if (userStripProgramNumber == 0) {
      updateChamberStatusLEDs(chamberStatus, 4);
    }
    lastStatusSendMillis = currentMillis;
  }
  else {
    // hasn't changed, if it's been a long time then re-tell the canbus
    // this may be overkill but its relatively cheap insurance in case
    // we are running long before roborio starts or is rebooting etc
    if ((currentMillis - lastStatusSendMillis) >= canRefreshMillis) {
      sendChamberStatus(chamberStatus, 4, myCanID);
      lastStatusSendMillis = currentMillis;
    }
  }
  // finally we do the fluff task of updating the LEDs for user programs
  // we only want to do that if there is NOT a fresh CAN message waiting for us
  // it's possible, for example,  that we got a fresh message while we were reading sensors
  // and setting feedbackasa LEDs -- processing that is more important animation or running
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
    // assume we can afford the time to show after we have done all the work
    strip.show();
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

    for (int i = 0; i < NUM_REV_LIGHT_SENSORS; i++) {
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
  int row;
  int col = 8;
  RGB color0;
  RGB color1;
  RGB colorEven;
  RGB colorOdd;
  Serial.println("strip zero running");
  unsigned long secs;
  color0.r = 0; color0.g = 0; color0.b = 128;
  color1.r = 128; color1.g = 0; color1.b = 0;
  secs = now / 1000;
  if ((secs % 2)) {
    colorEven = color0;
    colorOdd = color1;
  }
  else {
    colorOdd = color0;
    colorEven = color1;
  }
  for (row = 0; row < 8; row += 2) {
    matrixPutPixel(&strip, colorEven, col, row, 9, 8);
    matrixPutPixel(&strip, colorOdd, col, row + 1, 9, 8);
  }
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
  int userHigh = 72;
  int userLow = 64;
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
  int highLED = 72;
  int lowLED = 64;
  int snakeRed[15] = { 0, 20, 60, 120, 255,   120, 60, 20, 0, 0,  0, 0, 0, 0, 0 };
  int snakeGreen[15] = { 0, 0, 0, 0, 20, 60, 120, 255, 120, 60, 20, 0, 0, 0, 0 };
  int snakeBlue[15] = { 0, 0, 0, 0, 0, 0, 0, 20, 60, 120, 255, 120, 60, 20, 0 };
  static unsigned long lastUpdate = 0;
  unsigned long updateTime = 75L;
  static int offset = 0;

  if ((now - lastUpdate) > updateTime) {
    //Serial.println("prog 2");
    lastUpdate = now;
    for (int i = 0; i < (highLED - lowLED); i++) {
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
  if ((currentMillis - lastMillis) > totalCycle) lastMillis = currentMillis;
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
  } else {
    if ((currentMillis - lastMillis) < off) {
      stripBrightness = stripBrightness - 1;
      strip.setBrightness(stripBrightness);
    }
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
    watchdog = 1; // watchDogPixel matches the blinky lights
    if ((currentMillis - lastMillis) < off)
      strip.setPixelColor(watchDogPixel, 0, 0, 0);
    else
      strip.setPixelColor(watchDogPixel, 255, 120, 0);
  } else {
    strip.setPixelColor(watchDogPixel, 64, 20, 0); //1/4 brightness
  }
  //strip.show();
}

void sendChamberStatus(unsigned char* statusbytes, int numSensors, unsigned long canID) {
  int APIClassId = INCAN_CL_CHUTE;
  int APIIndex = 0;
  // Serial.println("sending chamber status");
  //Serial.println(statusbytes);
  canID = FRC_CAN_embed(canID, APIClassId, APIIndex);
  CAN.sendMsgBuf(canID, FRC_EXT, numSensors, statusbytes);
}

//int detectBalls(unsigned char *statusbytes) {
// read the sensors on I2c compare logical output to statusbytes
// if different then update statusbytes and return 1
// if no changes return 0;
//}

void updateChamberStatusLEDs(unsigned char* buf, int numChambers) {
  int chamberLow[4] = {6, 4, 2, 0};
  int chamberHigh[4] = {8, 6, 4, 2 };
  int colMin = 1;
  int colMax = 8;
  int ledPerColumn = 8;
  int rgb[3] = { 0, 0, 0 };
  int i;
  int  col;
  RGB color;
  for (i = 0; i < numChambers; i++) {
    switch (buf[i]) {
      case IDK:
        //Serial.print('chamber ');
        //Serial.print(i);
        //Serial.println(' is IDK');

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
    color.r = rgb[0];
    color.g = rgb[1];
    color.b = rgb[2];
    for (int row = chamberLow[i]; row < chamberHigh[i]; row++) {
      for (col = colMin; col < colMax; col++) {
        matrixPutPixel(&strip, color, col, row, 9, 8);
      }
      //   Serial.print(" content ");
      //   Serial.print(buf[i]);
      //   Serial.println(" done ");
    }
  }
  strip.show();
}
