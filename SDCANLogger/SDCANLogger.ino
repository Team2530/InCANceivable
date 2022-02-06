#include <SPI.h>
#include <SD.h>
#include <FRC_CAN_utils.h>
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>

// For Arduino MCP2515 Hat:
const int CAN_INT_PIN = 2;

// CAN is a global in the InCANceivable library (InCANceivable.cpp), so we mark it `extern`.
extern MCP_CAN CAN;
unsigned long myCanID; // Will later be set during initialization

File logFile; // File pointer to SD card
const char filename[30] = "canlog_0.csv";

void setup() {
  // Start a serial connection for local debugging
  // make sure to remove all `Serial` stuff when not connecting to the board over USB
  Serial.begin(115200);

  // Specific to the Seeed CAN shield - A pin is used to send a "interrupt" (very low-level message) to the board via a specific pin.
  // In the case of the Seeed CAN shield, this pin is 2
  // attachInterrupt registers a callback to be fired when a interrupt happens.
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING);

  // FRC_CAN_init() Initializes the CAN chip and generates the device's ID, which later can be used to check if messages are specifically meant for this device.
  // InCANceivable_setup() sets up all the proper masks, filters, etc. for sending and receiving data from the RIO and other devices.
  myCanID = FRC_CAN_init();
  InCANceivable_setup();

  // Logger specific - Initializes the SD card on the Seeed shield.
  if (!SD.begin(4)) {
      Serial.println("SD init fail!");
      while(true);
  } Serial.println("SD init OK.");

  // Make sure to create a new logfile.
  for (int logn = 0; SD.exists(filename); ++logn) {
    sprintf(filename, "canlog_%d.csv", logn);
  }
}

void loop() {
  static int canRunning = 0;
  static unsigned char lastHeartbeat[8] = { 0 };

  // Locals for storing data received from the CAN bus
  unsigned long canID;
  unsigned char CANbuf[8];
  unsigned char CANlen = 8;
  int messageAPIclass;
  int messageAPIindex;

  // CANflagRecv is set to 1 when the interrupt callback is ran.
  if (CANflagRecv) {
    CANflagRecv = 0; // Zero the flag
    logFile = SD.open(filename, FILE_WRITE);
    while (CAN_MSGAVAIL == CAN.checkReceive()) {
      // Read the message from buffers in the CAN chip
      CAN.readMsgBufID(&canID, &CANlen, CANbuf);

       // Discard first CAN packet
      if (canRunning == 0) {
        canRunning = 1;
      } else {
        // FRC_CAN_isMe(msgID, myID) tests if a message is "destined" for this device
        if (FRC_CAN_isMe(canID, myCanID)) {
          Serial.println("Message for me:");
          
          // FRC_CAN_extractClass gets the API class and API index (not used) from the CAN message
          FRC_CAN_extractClass(canID, &messageAPIclass, &messageAPIindex);
          
          // Pretty-prints a CAN message with all kinds of useful info
          InCANceivable_msg_dump(canID, CANlen, CANbuf);
        } else if (FRC_CAN_isRIO(canID)) {
          bool diff = false;
          for (int i = 0; i < CANlen; ++i) {
            diff |= lastHeartbeat[i] != CANbuf[i];
            lastHeartbeat[i] = CANbuf[i]; // Update last heartbeat
          }

          if (!diff) {
            continue; // We don't want to log repeated similar RIO heartbeats
          }
        }

        // -------- Logging --------
        
        logFile.print(millis()); // Timestamp
        logFile.print(",");
        logFile.print(canID); // ID
        logFile.print(",");
        logFile.print(messageAPIclass); // API class
        logFile.print(",");
        logFile.print(messageAPIindex); // API idx
        logFile.print(",");
        logFile.print(CANlen); // Data len

        // Data
        for (int i = 0; i < CANlen; ++i) {
          logFile.print(",");
          logFile.print(CANbuf[i]); 
        }
        
        logFile.println();
      }
    }
    logFile.close();
  }
}
