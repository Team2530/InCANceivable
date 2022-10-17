#ifndef AS5145_h
#define AS5145_h

#include "Arduino.h"

class AS5145
{
  public:
    AS5145(uint16_t DataPin, uint16_t ClockPin, uint16_t ChipSelectPin, uint16_t ProgramInputPin);	// for digital mode
    AS5145(uint16_t PWMPin);		// for pwm mode
    uint32_t encoder_degrees(void);	// get the absolute degree
    uint32_t encoder_value(void);	// get the raw data, 0-4095
    uint32_t encoder_error(void);	// have not implemented this function yet
    uint32_t pwm_degrees(void);		// get the absolute degree
    uint32_t high_value(void);		// get high time of a pulse
    uint32_t low_value(void);		// get low time of a pulse
    struct err_struct{
  	bool DECn;
	bool INCn;
	bool OCF;
	bool COF;
	bool LIN; } err_value;
  private:
    uint32_t read_chip(void);
    const uint16_t _clock;        // clock pin: output from arduino to AS5145
    const uint16_t _cs;           // chip select: output
    const uint16_t _data;         // data pin: input
    const uint16_t _pdio;	  // program input pin: input
    const uint16_t _pwm;	  // pwm input pin: input
    
};

#endif

