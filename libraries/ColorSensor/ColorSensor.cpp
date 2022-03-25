#include "ColorSensor.h"
#include <Arduino.h>
// Readings from the sensors when there isnt anything inside the chute.
static uint16_t baseProxReadings[8] = { 50, 50, 50, 50 }; // Add more if needed

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
    Wire.endTransmission();
    Wire.requestFrom(addr, (unsigned char)2);
    Wire.readBytes((uint8_t*)&d, 2);
    //Wire.endTransmission();
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

//  
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
	if (switchMux(i)){
          delayMicroseconds(10);
	  // oops maybe a problem switching the mux?  try one more time
	  switchMux(i);
	}
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
          if (prox>5000){
	    // Serial.print(baseProxReadings[i]);
	    //Serial.print(" ");
	    //Serial.println(prox);
	  }
	  if (prox < 5000) {
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

int detectBalls_prox(unsigned char* oldstates, int nsensors, void **proxPointers) {
  // warning this is not optimally organized  
    uint32_t channels[3];
    unsigned char states[8];
    VCNL4040* proximitySensor;
    unsigned int proxThreshold=40; 
    int ret = 0;
    unsigned long loopBegin;
    loopBegin=millis();
    for (int i = nsensors-1; i >= 0; i--) {
      int proxBallPresent=-1;
      //Serial.print(" processing prox sensor " );
      //Serial.print(i);
      //Serial.print(" ");
        // Sensor has been marked as IDK, which indicates there is some sort of problem.
        // So don't read it again
      if (oldstates[i] == BALL_IDK) {
	//  Serial.print("sensor ");
	// Serial.print( i );
	// Serial.println(" is IDK");
            continue;
        }
        if (proxPointers[i]){
	  int proxChannel=i+4;
	  unsigned int proxValue=0;
	  if (switchMux(proxChannel)){
	    //  Serial.println("mux switch problems");
	       delayMicroseconds(10);
	    switchMux(proxChannel);
	  }
          //delay(10);
	  //	  Serial.println((long)proxPointers[i],HEX);
	  //proximitySensor=(VCNL4040*) proxPointers[i];
	  proximitySensor=proxPointers[i];
	  proximitySensor->begin(Wire); // 20220228  Don't understand why we have to begin the proximitySensor 
	  //Serial.print("polling sensor ");
	  //Serial.print(i);
	  //Serial.print(" address is ");
	  //Serial.print( (long) proximitySensor, HEX);          
          //Serial.print( " is connected ");
	  //Serial.println(proximitySensor->isConnected());
          //proximitySensor->powerOnProximity();
          //Serial.println(" powered proximity on -- now read");
	  proxValue = proximitySensor->getProximity();
          //Serial.print(" prox value ");Serial.println(proxValue);
          if (proxValue>proxThreshold){
            proxBallPresent=1;
	    // Serial.println("prox ball present");
	  }
	  else {
	    //   Serial.println("prox ball absent");
	    proxBallPresent=0;
	  }
	}
        else {
	  proxBallPresent=-1;
	}
	// now if proxBallPresent ==  0 there is a proximity sensor on this channel and no ball
	//  if 1 then a ball is present
        //  if -1 there is a NO  proximity sensor for this channel
        //  or  
        if (proxBallPresent==0){  // then we don't even need to turn on the color sensor mux
	  if (oldstates[i] == BALL_IDK){  // at some point in the past the color sensor borked -- skip it. 
	    // we could have been smarter and not even read the prox sensor 
	    // it's unlikely to have a decommissioned color sensor so not fixing (JG 20220226)      
            states[i]=BALL_IDK;  
	    continue;
	  }
          else{
	    states[i]= BALL_NONE;
	  }
	} 
	else{
	  //          Serial.println("connecting color sensor");
	  // we either don't have a proximity sensor or we need to read the color sensors -- wire up the mux
	  if (switchMux(i)){
	    delayMicroseconds(10);
	    // oops maybe a problem switching the mux?  try one more time
	    switchMux(i);
	  }
          //Serial.print(i);
          //Serial.print(" ");
          //Serial.println(millis()-loopBegin);
          //delayMicroseconds(10);
	  if (proxBallPresent==-1){
	    // get the proximity reading from the color sensor
	    uint16_t colorProx;
	    uint16_t diff;
	    colorProx=getColorSensorProximity();
	    // diff=(colorProx-baseProxReadings[i]);
            diff=colorProx;
	    if (diff<0) 
	      colorProx=0;
	    else 
	      colorProx=diff;
	    // now measurement is relative to zero
            
	    //	    Serial.print(baseProxReadings[i]);
	    //Serial.print(" ");
	    //Serial.println(colorProx);
	    if (colorProx < 300) {
	      states[i] = BALL_NONE;          
	    } 
            else{
	      proxBallPresent=1;
	    }
	  } 
	  if (proxBallPresent==1) {
	    // ball is present by some proximity sensor somehow
	    bool err = getChannels(channels);
	    if (err) {
	      oldstates[i] = BALL_IDK;
	      ret=1;
	    } 
	    else{
	      channels[0]=channels[0]*0.7/0.9; 
	      // approximate kludge sensor is much hotter for red than blue
	      // see APDS-9151 data sheet plots 
	      //Serial.println(channels[1]);
	      if (channels[0] > channels[2]) {
		states[i] = BALL_RED;
	      }
	      else {
		states[i] = BALL_BLUE;
	      }
	    }
	  
	  }
	}
	// did this sensor change?
	if (states[i] != oldstates[i]){
	  ret = 1;
	}
	oldstates[i] = states[i];   
    }
    return(ret);
}

int detectBalls_prox_interrupt(unsigned char* oldstates, int nsensors, int *proxPins) {
  // warning this is not optimally organized  
  uint32_t channels[3];
  unsigned char states[8];
  int ret = 0;
  unsigned long loopBegin;
  loopBegin=millis();
  for (int i = nsensors-1; i >= 0; i--) {
    int proxBallPresent=-1;
    //Serial.print(" processing prox sensor " );
    //Serial.print(i);
    //Serial.print(" ");
    // Sensor has been marked as IDK, which indicates there is some sort of problem.
    // So don't read it again
    if (oldstates[i] == BALL_IDK) {
      //  Serial.print("sensor ");
      // Serial.print( i );
      // Serial.println(" is IDK");
      continue;
    }
    if (proxPins[i]>=0){
      if (digitalRead(proxPins[i])){
	  proxBallPresent=0;
	}
      else{    //   Serial.println("prox ball absent");
	proxBallPresent=1;
      }
    }
    else {
      proxBallPresent=-1;
    }
    // now if proxBallPresent ==  0 there is a proximity sensor on this channel and no ball
    //  if 1 then a ball is present
    //  if -1 there is a NO  proximity sensor for this channel
    //  or  
    if (proxBallPresent==0){  // then we don't even need to turn on the color sensor mux
      if (oldstates[i] == BALL_IDK){  // at some point in the past the color sensor borked -- skip it. 
	// we could have been smarter and not even read the prox sensor 
	// it's unlikely to have a decommissioned color sensor so not fixing (JG 20220226)      
	states[i]=BALL_IDK;  
	continue;
      }
      else{
	states[i]= BALL_NONE;
      }
    } 
    else{
      //          Serial.println("connecting color sensor");
      // we either don't have a proximity sensor or we need to read the color sensors -- wire up the mux
      if (switchMux(i)){
	delayMicroseconds(10);
	// oops maybe a problem switching the mux?  try one more time
	switchMux(i);
      }
      //Serial.print(i);
      //Serial.print(" ");
      //Serial.println(millis()-loopBegin);
      //delayMicroseconds(10);
      if (proxBallPresent==-1){
	// get the proximity reading from the color sensor
	uint16_t colorProx;
	uint16_t diff;
	colorProx=getColorSensorProximity();
	// diff=(colorProx-baseProxReadings[i]);
	diff=colorProx;
	if (diff<0) 
	  colorProx=0;
	else 
	  colorProx=diff;
	// now measurement is relative to zero
        
	//	    Serial.print(baseProxReadings[i]);
	//Serial.print(" ");
	//Serial.println(colorProx);
	if (colorProx < 300) {
	  states[i] = BALL_NONE;          
	} 
	else{
	  proxBallPresent=1;
	}
      } 
      if (proxBallPresent==1) {
	// ball is present by some proximity sensor somehow
	bool err = getChannels(channels);
	if (err) {
	  oldstates[i] = BALL_IDK;
	  ret=1;
	} 
	else{
	  channels[0]=channels[0]*0.7/0.9; 
	  // approximate kludge sensor is much hotter for red than blue
	  // see APDS-9151 data sheet plots 
	  //Serial.println(channels[1]);
	  if (channels[0] > channels[2]) {
	    states[i] = BALL_RED;
	  }
	  else {
	    states[i] = BALL_BLUE;
	  }
	}
	
      }
    }
    // did this sensor change?
    if (states[i] != oldstates[i]){
      ret = 1;
    }
    oldstates[i] = states[i];   
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

int switchMux(unsigned char channel, unsigned char mux_addr) {
  int ret;
  Wire.beginTransmission(mux_addr);
  Wire.write(1 << channel);
  // should check that the endTransmission() returned no errorts
  ret=Wire.endTransmission();
  return ret;
}

void calibrateBallDetection(int channel, unsigned char addr) {
  // moved which one up calling function -- we don't want to deal with keeping track
  // of how many sensors 
  //for (int i = 0; i < 2; ++i) {
  int proxValue=0;
  switchMux(channel);
  delay(50);
  proxValue = getColorSensorProximity();
  if (proxValue<200) {
    baseProxReadings[channel] = getColorSensorProximity();
  }
  // else proxValue is bogus -- run with default 
 
  //  Serial.print(channel) ;
  // Serial.print( " value");
  //Serial.println(baseProxReadings[channel]);
//       Serial.println(baseProxReadings[channel]);
 //
}
