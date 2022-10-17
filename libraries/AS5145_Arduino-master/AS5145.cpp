#include "Arduino.h"
#include "AS5145.h"

AS5145::AS5145(uint16_t DataPin, uint16_t ClockPin, uint16_t ChipSelectPin, uint16_t ProgramInputPin)
  : _data(DataPin), _clock(ClockPin), _cs(ChipSelectPin), _pdio(ProgramInputPin), _pwm(0) {
  pinMode(_data, INPUT);
  pinMode(_clock, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_pdio, OUTPUT);
}

AS5145::AS5145(uint16_t PWMPin) : _data(0), _clock(0), _cs(0), _pdio(0), _pwm(PWMPin) {
  pinMode(_pwm, INPUT);
}


uint32_t AS5145::encoder_degrees(void) {
  return ((encoder_value() * 360) / 4096);
}

uint32_t AS5145::encoder_value(void) {
  return (read_chip() >> 6);
}

uint32_t AS5145::pwm_degrees(void) {
  if (_pwm != 0) {
    int high_time = pulseIn(_pwm, HIGH);
    int low_time = pulseIn(_pwm, LOW);
    return (high_time * 4098.0 / (high_time + low_time) - 1);
  } else {
    return 0;
  }
}

uint32_t AS5145::high_value(void) {
  return pulseIn(_pwm, HIGH);
}

uint32_t AS5145::low_value(void) {
  return pulseIn(_pwm, LOW);
}

uint32_t AS5145::encoder_error(void) {
  uint16_t error_code;  // not yet implemented
  uint32_t raw_value;
  raw_value = read_chip();
  error_code = raw_value & 0b0000000000111111;
  //err_value.DECn = error_code & 2;
  //err_value.INCn = error_code & 4;
  err_value.LIN = error_code & 8;
  err_value.COF = error_code & 16;
  err_value.OCF = !(error_code & 32);
  return error_code;
}

uint32_t AS5145::read_chip(void) {
  uint32_t raw_value = 0;
  uint16_t inputstream = 0;
  uint16_t c;
  digitalWrite(_pdio, LOW);
  digitalWrite(_cs, HIGH);
  digitalWrite(_clock, HIGH);
  digitalWrite(_cs, LOW);
  delayMicroseconds(50);
  digitalWrite(_clock, LOW);
  // read 18 bits
  for (c = 0; c < 18; c++) {
    digitalWrite(_clock, HIGH);
    delayMicroseconds(5);
    inputstream = digitalRead(_data);
    raw_value = ((raw_value << 1) + inputstream);
    digitalWrite(_clock, LOW);
    delayMicroseconds(5);
  }
  digitalWrite(_cs, HIGH);
  return raw_value;
}


