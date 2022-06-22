# InCANceivable

### FRC Team 2530's framework for programming embedded systems.

InCANceivable is our teams' libraries and Arduino sketches for small independent daughterboards (Arduinos) connected to the CAN bus as accessories/coprocessors.
We are using InCANceivable this season (2022 Rapid React) to run multiple color sensors, proximity sensors, and provide visual feedback from the robot with 2 large LED displays.

## Hardware

InCANceivable's core is reliant on the MCP2515 SPI CAN controller, and has been used with 2 devices so far, the [Longan Labs CANBed](https://www.longan-labs.cc/1030008.html) and the [Seeed CAN shield](https://www.seeedstudio.com/CAN-BUS-Shield-V2.html). A small library is also provided for using the [REV Robotics Color Sensor V3](https://www.revrobotics.com/rev-31-1557/).

## Compiling

The Arduino IDE normally looks for sketches in a certain sketchbook folder where libraries and sketches are stored. InCANceivable uses custom-made libraries, so you can't just copy InCANceivable's sketches into your sketchbook. 
We recommend that you clone this repo somewhere else convenient and set the Arduino IDE's sketchbook path to the root folder of this repository. After restarting the IDE, the sketches should show up in the menu, and the libraries should be available.

## Usage

See inside each sketch's folder to view specific operation instructions for each. At this time, not all included sketches in this repository are documented.

## Companion Java code

A library for our Java robot code has been made, you can find it in our team's robot code for this year: [InCANDevice.java](https://github.com/Team2530/RobotCode2022/blob/main/src/main/java/frc/robot/InCANDevice.java) (Base class for device interfacing), [Chambers.java](https://github.com/Team2530/RobotCode2022/blob/main/src/main/java/frc/robot/subsystems/Chambers.java) (Receives ball sensor data from the device), [Indicators.java](https://github.com/Team2530/RobotCode2022/blob/main/src/main/java/frc/robot/subsystems/Indicators.java) (Controls programmable indicators), and [FeedbackPanel.java](https://github.com/Team2530/RobotCode2022/blob/main/src/main/java/frc/robot/subsystems/FeedbackPanel.java) (For displaying images).

## Compliance with FRC CAN bus specification

The `ChuteManager_3` and `SDCANLogger` programs are currently used for our robot in competition. 

SDCANLogger does not control everything, and does not respond to CAN frames, it is simply a passive listener.

`ChuteManager_3` controls lights, sensors, and responds to halt broadcasts and absent heartbeat. 

Our device transmits and receives on a fixed address

- Manufacturer 8 (team use)

- Device type 10 (misc)

- Device number (default) 3
  
  - Pins 10 and 9 on the device can be pulled down to turn off bits in the device number, by default both pins are pulled high with an internal resistor.

- Using the following API classes (note: in WPILib robot code, API *class* must be shifted left 4 to convert it into a API *ID* used by the WPILib CAN API because our device doesn't use API indices)
  
  - 32 is used when sending messages to the RIO
    
    - One byte per sensor
  
  - 25 is used to control indicators on the LED panel
    
    - Two 24-bit RGB triplets, one for each indicator
  
  - 28 is used to display indexed images on the LED panel
    
    - Single byte for index

The device also receives messages from the PDP regarding battery level, and displays a battery gauge.

![](ChuteManager_3/feedbackpanel-layout.svg "LED panel layout")

See [the documentation for ChuteManager_3](ChuteManager_3/README.md) for more information.
