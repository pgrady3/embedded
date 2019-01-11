#include "Arduino.h"
#include "Audio.h"

#define AUDIO_PIN     9

void AudioInit(void)
{
  pinMode(AUDIO_PIN, OUTPUT);
  digitalWrite(AUDIO_PIN, LOW);
  
  TCCR1A = (1<<6);//toggle pin 9
  TCCR1B = (1<<3);//ctc
  TCCR1C = 0;
  TCNT1 = 0;
  OCR1A = 1000;
  OCR1B = 0;
  OCR1C = 0;
  //TIMSK1 = (1<<1);//compa interrupt
  TCCR1B |= (1<<1);
  
  AudioSetFreq(1000);

}

void AudioSetFreq(uint16_t freq)
{
  uint16_t ocr = 1000000 / freq - 1;
  uint16_t tmr = TCNT1 + 50;
  OCR1A = ocr;

  if(TCNT1 > ocr)
  {
    TCNT1 = 0;
    TCCR1C = (1<<7);
  }
}

void AudioOn(void)
{
  TCCR1A = (1<<6);//toggle pin 9
}

void AudioOff(void)
{
  TCCR1A = 0;
}

