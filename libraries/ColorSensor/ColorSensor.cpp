#include "ColorSensor.h"
#include <Arduino.h>
// Readings from the sensors when there isnt anything inside the chute.
static uint16_t baseProxReadings[8] = { 0, 0 }; // Add more if needed

uint32_t read20BitRegister(unsigned char addr, ColorSensorRegister reg, bool* err) {
    uint32_t data;

    Wire.beginTransmission(addr);
    Wire.write((unsigned char)reg);
    if (err != NULL)
        *err = Wire.endTransmission(false);
    else
        Wire.endTransmission(false);

    // Request 3 uchars (24 bits)
    Wire.requestFrom(addr, (unsigned char)3);
    Wire.readBytes((uint8_t*)&data, 3);

    // Only use 20 of the 24 bits? Not sure if I get why. I also dont get why the mask is only 17 bits...
    return (data & 0x03FFFFL);
}

// Returns true on error
bool write8BitRegister(unsigned char addr, ColorSensorRegister reg, unsigned char data) {
    Wire.beginTransmission(addr);
    Wire.write((unsigned char)reg);
    Wire.write(data);
    return Wire.endTransmission() != 0;
}

bool initColorSensor(unsigned char addr) {
    bool err = write8BitRegister(
        addr,
        ColorSensorRegister::MainCtrl,
        ColorSensorMainControlFlags::RGBMode
        | ColorSensorMainControlFlags::LightSensorEnable
        | ColorSensorMainControlFlags::ProximitySensorEnable);

    err |= write8BitRegister(
        addr,
        ColorSensorRegister::ProximitySensorRate,
        ColorSensorProximityResolution::Res11bit
        | ColorSensorMeasurementRate::Rate100ms);

    err |= write8BitRegister(addr, ColorSensorRegister::ProximitySensorPulses, (unsigned char)32);

    return err;
}

uint16_t getColorSensorProximity(unsigned char addr) {
    uint16_t d;

    Wire.beginTransmission(addr);
    Wire.write(ColorSensorRegister::ProximityData);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, (unsigned char)2);
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
    int ret = 0;
    for (int i = 0; i < nsensors; ++i) {
        // Sensor has been marked as IDK, which indicates there is some sort of problem.
        // So don't read it again
        if (oldstates[i] == BALL_IDK) {
	  //Serial.print("sensor ");
          //Serial.print( i );
	  //Serial.println(" is IDK");
            continue;
        }
        switchMux(i);
        bool err = getChannels(channels);
	if (err) {
	  oldstates[i] = BALL_IDK;
          ret=1;

        } 
	else{
          uint16_t prox;
          int16_t diff;
          channels[0]=channels[0]*0.5/0.9; 
	  // approximate kludge sensor is much hotter for red than blue
	  // see APDS-9151 data sheet plots 
	  //Serial.println(channels[1]);
	  prox=getColorSensorProximity();
	  diff=(prox-baseProxReadings[i]);
          if (diff<0) 
	    prox=0;
	  else 
	    prox=diff;
	  
	  // now measurement is relative to zero
	  // Serial.print(baseProxReadings[i]);
          //Serial.print(" ");
	  //Serial.println(prox);
	  if (prox < 50) {
	    states[i] = BALL_NONE;          
	  } 
	  else {if (channels[0] > channels[2]) {
	      states[i] = BALL_RED;
	    } else {
	      states[i] = BALL_BLUE;
	    }
	  }
	  if (states[i] != oldstates[i]) ret = 1;
	  oldstates[i] = states[i];
	  }
    }
    return(ret);
}

bool getChannels(uint32_t* rgb, unsigned char addr) {
  bool err[3] = {false,false,false};
  bool ret=false;
    rgb[0] = read20BitRegister(addr, ColorSensorRegister::DataRed, err);
    rgb[1] = read20BitRegister(addr, ColorSensorRegister::DataGreen, err+1);
    rgb[2] = read20BitRegister(addr, ColorSensorRegister::DataBlue, err+2);
    ret= (err[0] | err[1] | err[2]);
    return ret;
}

void switchMux(unsigned char channel, unsigned char mux_addr) {
    Wire.beginTransmission(mux_addr);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

void calibrateBallDetection(int channel, unsigned char addr) {
  // moved which one up calling function -- we don't want to deal with keeping track
  // of how many sensors 
  //for (int i = 0; i < 2; ++i) {
        switchMux(channel);
        baseProxReadings[channel] = getColorSensorProximity();
	//       Serial.println(baseProxReadings[channel]);
 //
}
