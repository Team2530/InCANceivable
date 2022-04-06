# InCANceivable

### FRC team 2530's framework for programming embedded systems.

InCANceivable is our teams libraries and Arduino sketches for small independent daughterboards (Arduinos) connected to the CAN bus as accessories/coprocessors.
We are using InCANceivable this season (2022 Rapid React) to run multiple color sensors, proximity sensors, and provide visual feedback from the robot with 2 large LED displays.

## Hardware

InCANceivable's core is reliant on the MCP2515 SPI CAN controller, and has been used with 2 devices so far, the [Longan Labs CANBed](https://www.longan-labs.cc/1030008.html) and the [Seeed CAN shield](https://www.seeedstudio.com/CAN-BUS-Shield-V2.html). A small library is also provided for using the [REV Robotics Color Sensor V3](https://www.revrobotics.com/rev-31-1557/).

## Compiling

The Arduino IDE normally looks for sketches in a certain sketchbook folder where libraries and sketches are stored. InCANceivable uses custom-made libraries, so you can't just copy InCANceivable's sketches into your sketchbook. 
We reccomend you clone this repo somewhere else convenient and set the Arduino IDE's sketchbook path to the root folder of this repository. After restarting the IDE, the sketches should show up in the menu, and the libraries should be available.

## Rules compliance/Inspection

Please see [Compliance with FRC CAN bus specification](./FRC-CAN-Compliance.md).
