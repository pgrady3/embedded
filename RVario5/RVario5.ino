#include <Wire.h>
#include <SPI.h>
#include "Baro.h"
#include "Radio.h"
#include "Audio.h"
#include "Input.h"

#define SINK_DEADBAND -0.01
#define LIFT_DEADBAND 0.01

void setup() {
  Serial.begin(9600);     //Begin USB serial
  delay(5000);            //Wait for PC to load. 5sec is required
  BaroInit();
  RadioInit();
  AudioInit();
  
  idleDelay(3000);
  AudioOff();
}

void loop() {
  float velo = BaroGetVelo();
  
  if(velo < -10)    velo = -10;
  if(velo > 10)     velo = 10;
  
  if(velo < SINK_DEADBAND)
  {
    AudioSetFreq(350 * pow(1.25, velo));
    AudioOn();
    idleDelay(10);
  }
  else if(velo > LIFT_DEADBAND)
  {
    AudioSetFreq(500 * pow(1.25, velo));
    AudioOn();
    idleDelay(300.0 / (2 + velo));
    AudioOff();
    idleDelay(300.0 / (2 + velo));
  }
  else
  {
    AudioOff();
    idleDelay(200);
  }
}

void idleDelay(uint16_t ms)//do idle tasks until time is hit
{
  uint32_t endTime = millis() + ms;

  while(millis() < endTime)
  {
    BaroPoll();
  } 
}
