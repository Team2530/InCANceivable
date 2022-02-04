#include "ColorSensor.h"

// Readings from the sensors when there isnt anything inside the chute.
extern uint16_t baseProxReadings[2] = { 0, 0 }; // Add more if needed

uint32_t read20BitRegister(unsigned char addr, ColorSensorRegister reg) {
    uint32_t data;

    Wire.beginTransmission(addr);
    Wire.write((unsigned char)reg);
    Wire.endTransmission(false);

    // Request 3 uchars (24 bits)
    Wire.requestFrom(addr, (unsigned char)3);
    Wire.readBytes((uint8_t*)&data, 3);

    // Only use 20 of the 24 bits? Not sure if I get why. I also dont get why the mask is only 17 bits...
    return data & 0x03FFFF;
}

void write8BitRegister(unsigned char addr, ColorSensorRegister reg, unsigned char data) {
    Wire.beginTransmission(addr);
    Wire.write((unsigned char)reg);
    Wire.write(data);
    Wire.endTransmission();
}

void initColorSensor(unsigned char addr) {
    write8BitRegister(
        addr,
        ColorSensorRegister::MainCtrl,
        ColorSensorMainControlFlags::RGBMode
        | ColorSensorMainControlFlags::LightSensorEnable
        | ColorSensorMainControlFlags::ProximitySensorEnable);

    write8BitRegister(
        addr,
        ColorSensorRegister::ProximitySensorRate,
        ColorSensorProximityResolution::Res11bit
        | ColorSensorMeasurementRate::Rate100ms);

    write8BitRegister(addr, ColorSensorRegister::ProximitySensorPulses, (unsigned char)32);
}

uint16_t getColorSensorProximity(unsigned char addr) {
    uint16_t d;

    Wire.beginTransmission(addr);
    Wire.write(ColorSensorRegister::ProximityData);
    Wire.endTransmission(false);
    Wire.requestFrom(addr,(unsigned char) 2);
    Wire.readBytes((uint8_t*)&d, 2);

    return d & 0x7FF; // Mask to 11 bits
}

Color getColorSensorColor(unsigned char addr) {
    float r = float(read20BitRegister(addr, ColorSensorRegister::DataRed));
    float g = float(read20BitRegister(addr, ColorSensorRegister::DataGreen));
    float b = float(read20BitRegister(addr, ColorSensorRegister::DataBlue));
    float l = r + g + b;

    Color c;
    c.r = r / l;
    c.g = g / l;
    c.b = b / l;

    return c;
}

int detectBalls(unsigned char* oldstates, int nsensors) {
    uint32_t channels[3];
    unsigned char states[8];
    int ret=0;
    for (int i = 0; i < nsensors; ++i) {
        switchMux(i);
        getChannels(channels);
        if (getColorSensorProximity() < baseProxReadings[i]/2) {
            states[i] = BALL_NONE;
        } else if (channels[0] > channels[2]) {
            states[i] = BALL_RED;
        } else {
            states[i] = BALL_BLUE;
        }
	if (states[i]!=oldstates[i]) ret=1;
        oldstates[i]=states[i];
    }
    
    return(ret);
}

void getChannels(uint32_t* rgb, unsigned char addr) {
    rgb[0] = read20BitRegister(addr, ColorSensorRegister::DataRed);
    rgb[1] = read20BitRegister(addr, ColorSensorRegister::DataGreen);
    rgb[2] = read20BitRegister(addr, ColorSensorRegister::DataBlue);
}

void switchMux(unsigned char channel, unsigned char mux_addr) {
    Wire.beginTransmission(mux_addr);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

void calibrateBallDetection(unsigned char addr) {
    for (int i = 0; i < 2; ++i) {
        switchMux(i);
        baseProxReadings[i] = getColorSensorProximity();
    }
}
