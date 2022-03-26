#include <SPI.h>
#include <SD.h>
#include <FRC_CAN_utils.h>
#include <InCANceivable_globals.h>
#include <InCANceivable.h>
#include <FRC_CAN.h>
#include <FRC_CAN_utils.h>
#include <stdio.h>

// For Arduino MCP2515 Hat:
const int CAN_INT_PIN = 2;

// CAN is a global in the InCANceivable library (InCANceivable.cpp), so we mark it `extern`.
extern MCP_CAN CAN;

File logFile; // File pointer to SD card
static char filename[30] = "log_0.csv";

void setup() {
  // Start a serial connection for local debugging
  // make sure to remove all `Serial` stuff when not connecting to the board over USB
  //Serial.begin(115200);

  // Specific to the Seeed CAN shield - A pin is used to send a "interrupt" (very low-level message) to the board via a specific pin.
  // In the case of the Seeed CAN shield, this pin is 2
  // attachInterrupt registers a callback to be fired when a interrupt happens.
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING);

  // Initialize the CAN chip
  while (CAN_OK != CAN.begin(FRC_CAN_SPEED)) delay(100);

  // Clear all masks.
  CAN.init_Mask(0,FRC_EXT,0);
  CAN.init_Mask(1,FRC_EXT,0);
  // No filters either.
  CAN.init_Filt(0,FRC_EXT,0);
  CAN.init_Filt(1,FRC_EXT,0);
  CAN.init_Filt(2,FRC_EXT,0);

  // Logger specific - Initializes the SD card on the Seeed shield.
  // 4 is the SPI chip select for the SD card
  while (!SD.begin(4)) delay(100);

  // Make sure to create a new logfile.
  for (int logn = 0; SD.exists(filename); ++logn) {
    sprintf(filename, "log_%d.csv", logn);
  }
  //Serial.println(filename);

  logFile = SD.open(filename, FILE_WRITE);
  logFile.println("timestamp_millis,can_id,data_length,data_0,data_1,data_2,data_3,data_4,data_5,data_6,data_7");
  logFile.close();
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
        if (FRC_CAN_isRIO(canID)) {
          bool diff = false;
          // Compare heartbeat bytes.
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
        logFile.print(CANlen); // Data len

        // Data
        for (int i = 0; i < 8; ++i) {
          logFile.print(",");
          if (i <= CANlen)
            logFile.print(CANbuf[i]); 
          else
            logFile.print(0); // Pad with zeros for equal columns
        }
        
        logFile.println();
      }
    }
    logFile.close();
  }
}
