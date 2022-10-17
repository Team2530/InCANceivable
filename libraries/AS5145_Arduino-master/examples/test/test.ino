#include <AS5145.h>

AS5145 myAS5145(18,15,16,17); // data, clock, chip select, program input.
//AS5145 myAS5145(9);       // pwm input

long step = 0;
const uint8_t P_LR = 13; //red led

void setup()
{
  pinMode(P_LR, OUTPUT);
  digitalWrite(P_LR, LOW);
  Serial.begin(9600);
}

boolean led_status = false;

void loop()
{
  Serial.print("Step: ");
  Serial.println(++step);
  led_status = !led_status;
  digitalWrite(P_LR, led_status);  // LED blinking
  long value;

  // method 1
  value = myAS5145.encoder_value();
  Serial.print("measured value: ");
  Serial.println(value);
  value = myAS5145.encoder_degrees();
  Serial.print("measured degrees: ");
  Serial.println(value);
  if (myAS5145.encoder_error()>>3)
  {
//    if (myAS5145.err_value.DECn) Serial.println("DECn error");
//    if (myAS5145.err_value.INCn) Serial.println("INCn error");
    if (myAS5145.err_value.COF) Serial.println("COF error");
    if (myAS5145.err_value.OCF) Serial.println("OCF error");
    if (myAS5145.err_value.LIN) Serial.println("out of range");
  }


  // method 2: pwm
//  value = myAS5145.high_value();
//  Serial.print("high time value: ");
//  Serial.println(value);
//  value = myAS5145.low_value();
//  Serial.print("low time value: ");
//  Serial.println(value);
//  value = myAS5145.pwm_degrees();
//  Serial.print("measured degrees: ");
//  Serial.println(value);
  
  delay(20);
}
 





