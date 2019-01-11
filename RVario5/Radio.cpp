    #include "Arduino.h"
#include "Baro.h"
#include <SPI.h>

//GPIO0 is connected to TXANT
//GPIO1 is connected to RXANT

#define MOSI  16
#define MISO  14
#define CS    18
#define SCK   15
#define SDN   19

#define FREQ_OFFSET  20

uint8_t RadioReadReg(uint8_t addr);
void RadioWriteReg(uint8_t addr, uint8_t data);
void RadioSetChannel(uint8_t ch);

void RadioSetChannel(uint8_t ch)
{
  uint16_t fc = 0;
  fc = 16560;
  
  fc += FREQ_OFFSET;

  uint8_t temp = fc >> 8;
  RadioWriteReg(0x76, temp);
  temp = fc & 0xFF;
  RadioWriteReg(0x77, temp);
}

void RadioInit(void)
{
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  SPI.begin();

  pinMode(SDN, OUTPUT);
  digitalWrite(SDN, HIGH);
  delay(200);
  digitalWrite(SDN, LOW);
  delay(200);
  
  if(RadioReadReg(0x01) != 0x06)
  {
    delay(100);
    Serial.println("BAD RADIO");
  }
  
  RadioWriteReg(0x07, 0x80);//set the reset bit
  delay(500);//and wait for the reset to happen

  //PLAY WITH LOAD CAPACITANCE
  //RadioWriteReg(0x09, 0x7F);//set
  
  RadioWriteReg(0x0B, 0b11101);//set GPIO0 and TXANT high
  RadioWriteReg(0x0C, 0b11111);//set GPIO1 and RXANT low
  RadioWriteReg(0x0D, 0b10000);//set GPIO2 as RF input
  
  RadioWriteReg(0x30, 0x00);//clear all packet handling
  RadioWriteReg(0x6D, 0x00);//min power
  //RadioWriteReg(0x6D, 0x07);//max power
  
  RadioWriteReg(0x71, 0x02);//fsk
  RadioWriteReg(0x72, 0x02);//deviation
  RadioWriteReg(0x75, 0x16);//select 460-470 band


  RadioSetChannel(2);

  //while(1)    delay(100);
  RadioWriteReg(0x07, 0x02);//TUNE mode
  delay(100);
  RadioWriteReg(0x07, 0x08);//TUNE mode
}

uint8_t RadioReadReg(uint8_t addr)
{
  addr &= ~(1<<7);//clear the read bit
  digitalWrite(CS, LOW);
  SPI.transfer(addr);
  uint8_t ret = SPI.transfer(0xAA);
  digitalWrite(CS, HIGH);
  return ret;
}

void RadioWriteReg(uint8_t addr, uint8_t data)
{
  addr |= (1<<7);//set the write bit
  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(addr);
  delayMicroseconds(10);
  SPI.transfer(data);
  digitalWrite(CS, HIGH);
  delayMicroseconds(10);
}

