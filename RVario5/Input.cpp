#include "Arduino.h"
#include "Input.h"

#define SW1 8
#define SW2 7
#define SW3 6
#define SW4 5
#define SW5 4
#define SW6 10

//note that a low value indicates that the switch is on
//also, i messed up the sw1-6 order, so its backwards from the schema

void InputInit()
{
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);
}

uint8_t SWRCEN()
{
  if(digitalRead(SW1))
    return 0;
  else
    return 1;
}

uint8_t SWDeadbandEN()
{
  if(digitalRead(SW2))
    return 0;
  else
    return 1;
}

uint8_t SWHighSensEN()
{
  if(digitalRead(SW3))
    return 0;
  else
    return 1;
}

uint8_t SWSinkEN()
{
  if(digitalRead(SW4))
    return 0;
  else
    return 1;
}

uint8_t SWChannel()
{
  uint8_t pos = digitalRead(SW6) << 1;
  pos |= digitalRead(SW5);

  return pos % 4;  
}

