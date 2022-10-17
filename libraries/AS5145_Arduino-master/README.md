# AS5145_Arduino
It is a library for AS5145 Magnetic Encoder

There are modes you can use, a digital one and a pwm one. The error massage has not been implemented yet.
  



## **Digital Mode**

Pin connection: need to connect DO (data), CLK (clock), CS (chip select) and Prg(program input) pins to the Arduino board.
Use AS5145(uint16_t DataPin, uint16_t ClockPin, uint16_t ChipSelectPin, uint16_t ProgramInputPin); to initialize.
And simply use the encoder_degrees() function to get the absolute degree from AS5145.

  e.g. int value = myAS5145.encoder_degrees();

For debugging, you can use the encoder_value() function to get the raw value.





## **PWM Mode**

I have not tested this mode yet.
Pin connection: only need to connect PWM pin to the Arduino board.
Simply call pwm_degrees() function to get the absolute degree.

  e.g. int value = myAS5145.pwm_degrees();

For debugging, you can use high_value() and low_value().



**A little suggestion: if your kart runs at 15 miles per hour with the diameter 6 inches of your wheel, the period to get angle value must be less than 70 ms.**


I have also attached a test file.
