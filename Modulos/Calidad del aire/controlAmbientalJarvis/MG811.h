/*
MG811.h - Library for MG811 sensor(COO sensor).
Created by CrakerNano, May 18, 2015. Released into the public domain.
*/
#ifndef MQ811_h
#define MQ811_h
#include "Arduino.h"

class MG811{
   public: MG811(int pin); 
   float getCOO();

   private:
   int _pin;
   int val;
   float ppm;
};

#endif
