#ifndef COLORSENSOR_H
#define COLORSENSOR_H

#include <Wire.h>
#include <SparkFun_VCNL4040_Arduino_Library.h>
#define BALL_IDK 0
#define BALL_RED 1
#define BALL_GREEN 2
#define BALL_BLUE 3
#define BALL_NONE 4

extern uint16_t baseProxReadings[];


typedef struct Color {
    float r, g, b;
} Color;

enum ColorSensorRegister {
    MainCtrl = (unsigned char)0x00,
    ProximitySensorLED = 0x01,
    ProximitySensorPulses = 0x02,
    ProximitySensorRate = 0x03,
    LightSensorMeasurementRate = 0x04,
    LightSensorGain = 0x05,
    PartID = 0x06,
    MainStatus = 0x07,
    ProximityData = 0x08,
    DataInfrared = 0x0A,
    DataGreen = 0x0D,
    DataBlue = 0x10,
    DataRed = 0x13,
};

enum ColorSensorMainControlFlags {
    RGBMode = (unsigned char)0x04,         /* "If bit is set to 1, color channels are activated" */
    LightSensorEnable = 0x02,     /* "Enable light sensor" */
    ProximitySensorEnable = 0x01, /* "Proximity sensor active" */
    OFF = 0x00                    /* "Nothing on" */
};

enum ColorSensorGainFactor {
    Gain1x = (unsigned char)0x00,
    Gain3x = 0x01,
    Gain6x = 0x02,
    Gain9x = 0x03,
    Gain18x = 0x04
};

enum ColorSensorLEDCurrent {
    Pulse2mA = (unsigned char)0x00,
    Pulse5mA = 0x01,
    Pulse10mA = 0x02,
    Pulse25mA = 0x03,
    Pulse50mA = 0x04,
    Pulse75mA = 0x05,
    Pulse100mA = 0x06, /* default value */
    Pulse125mA = 0x07
};

enum ColorSensorLEDPulseFrequency {
    Pulse60kHz = 0x18, /* default value */
    Pulse70kHz = 0x40,
    Pulse80kHz = 0x28,
    Pulse90kHz = 0x30,
    Pulse100kHz = 0x38
};

enum ColorSensorProximityResolution {
    Res8bit = 0x00,
    Res9bit = 0x08,
    Res10bit = 0x10,
    Res11bit = 0x18
};

enum ColorSensorResolution {
    Res20bit = 0x00,
    Res19bit = 0x10,
    Res18bit = 0x20,
    Res17bit = 0x30,
    Res16bit = 0x40,
    Res13bit = 0x50
};

enum ColorSensorMeasurementRate {
    Rate25ms = 0,
    Rate50ms = 1,
    Rate100ms = 2,
    Rate200ms = 3,
    Rate500ms = 4,
    Rate1000ms = 5,
    Rate2000ms = 7
};

const unsigned char COLORSENSORV3_ADDR = 0x52; // Standard address for REV color sensor V3

uint32_t read20BitRegister(unsigned char addr, ColorSensorRegister reg, bool* err = NULL);
bool write8BitRegister(unsigned char addr, ColorSensorRegister reg, unsigned char data);
bool initColorSensor(unsigned char addr = COLORSENSORV3_ADDR);
uint16_t getColorSensorProximity(unsigned char addr = COLORSENSORV3_ADDR);
Color getColorSensorColor(unsigned char addr = COLORSENSORV3_ADDR);
int detectBalls(unsigned char* states, int nsensors = 2);
int detectBalls_prox(unsigned char* states, int nsensors = 2, void** proxSensors=NULL);
bool getChannels(uint32_t* rgb, unsigned char addr = COLORSENSORV3_ADDR);
int switchMux(unsigned char channel, unsigned char mux_addr = 0x70);
void calibrateBallDetection(int sensorNumber=0, unsigned char channel = COLORSENSORV3_ADDR);

#endif
