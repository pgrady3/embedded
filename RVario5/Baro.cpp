#include "Arduino.h"
#include <Wire.h>
#include "Baro.h"

#define I2C_ADDR    0x76
#define DERIV_HISTORY_LEN   50
#define ALT_HISTORY_LEN     16
#define SAMPLES_SEC   52

//Function prototpyes
uint32_t  BaroGetProm(uint8_t promAddr);
void      BaroReset(void);
uint32_t  BaroGetConv(void);
void      BaroUpdate(uint32_t D1, uint32_t D2);

//Local variables
uint32_t prom[7];
uint8_t convertTemp = 0;
uint32_t convertTime = 0;
float alt = 0;
float derivHistory[DERIV_HISTORY_LEN];
float altHistory[ALT_HISTORY_LEN];
uint8_t altHistoryPos = 0;
uint8_t derivHistoryPos = 0;
float velo = 0;

void BaroPoll(void)
{
  static uint32_t D1 = 0;
  static uint32_t D2 = 0;
  static uint32_t avgD2 = 0;
  static uint32_t secondAvgD2 = 0;
  static uint32_t avgD1 = 0;
  
  if(convertTemp)                   //Currently doing a temp conversion
  {
    if(millis() < convertTime + 8)  //our conversion hasn't finished yet
      return;

    convertTime = millis();
    D2 = BaroGetConv();

    if(avgD2 == 0)
    {
      avgD2 = D2;
      secondAvgD2 = D2;
    }
    else
    {
      avgD2 += ((int64_t)D2 - avgD2) / 16;
      //secondAvgD2 += ((int64_t)D2 - secondAvgD2) / 32;
    }
    
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x48);               //Start D1 conversion, OSR=4096
    Wire.endTransmission();
    convertTemp = 0;
  }
  else                              //Doing a pressure conversion
  {
    if(millis() < convertTime + 12)  //our conversion hasn't finished yet
      return;

    convertTime = millis();
    D1 = BaroGetConv();

    if(avgD1 == 0)
      avgD1 = D1;
    else
      avgD1 += ((int64_t)D1 - avgD1) / 16;
    
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x56);               //Start D2 conversion, OSR=2048
    Wire.endTransmission();

    BaroUpdate(avgD1, avgD2);
    /*Serial.print(alt);
    Serial.print(" ");
    BaroUpdate(avgD1, secondAvgD2);
    Serial.println(alt);*/
    
    convertTemp = 1;
  }
}

void BaroUpdate(uint32_t D1, uint32_t D2)
{
  int32_t dT = D2 - (prom[5] << 8);
  int32_t Temp = (dT * prom[6]) >> 23;
  Temp += 2000;
  
  int64_t Off = ((int64_t)prom[2]) << 17;
  Off += (prom[4] * (int64_t)dT) / 64;
  int64_t Sens = (prom[1] << 16) + (prom[3] * (int64_t)dT) / 128;
  float P = ((D1 * Sens) / 2097152 - Off) / 32768.0;

  if(P < 100)
    return;
  
  alt = 44330 * (1 - pow((float)P / 101325.0f, 1.0f/5.255f));
  altHistory[altHistoryPos++] = alt;
  altHistoryPos %= ALT_HISTORY_LEN;

  float avgAlt = 0;
  for(uint8_t i = 0; i < ALT_HISTORY_LEN; i++)
    avgAlt += altHistory[i];

  avgAlt /= ALT_HISTORY_LEN;
  
  velo = (avgAlt - derivHistory[derivHistoryPos]) / DERIV_HISTORY_LEN * SAMPLES_SEC;
  derivHistory[derivHistoryPos++] = avgAlt;
  derivHistoryPos %= DERIV_HISTORY_LEN;  

  Serial.print(millis());
  Serial.print("\t");
  Serial.print(D1);
  Serial.print("\t");
  Serial.print(D2);
  Serial.print("\t");
  Serial.print(alt);
  Serial.print("\t");
  Serial.print(avgAlt);
  Serial.print("\t");
  Serial.println(velo);
}

uint32_t BaroGetConv(void)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 3);
  uint32_t ans = 0;
  ans += (uint32_t)Wire.read() << 16;
  ans += (uint32_t)Wire.read() << 8;
  ans += (uint32_t)Wire.read();
  return ans;
}

void BaroInit(void)
{
  for(uint8_t i = 0; i < DERIV_HISTORY_LEN; i++)
    derivHistory[i] = 0;

  for(uint8_t i = 0; i < ALT_HISTORY_LEN; i++)
    altHistory[i] = 0;
  
  Wire.begin();
  BaroReset();

  for(uint8_t i = 0; i < 7; i++)
    prom[i] = BaroGetProm(i);

  Serial.println("MS5637 PROM values");
  for(uint8_t i = 0; i < 7; i++)
  {
    Serial.print(i);
    Serial.print(':');
    Serial.println(prom[i], HEX);
  }
}

void BaroReset(void)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x1E);
  Wire.endTransmission();
  delay(100);
}

uint32_t BaroGetProm(uint8_t promAddr)
{
  promAddr %= 7;
  promAddr = 0xA0 | (promAddr << 1);

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(promAddr);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  uint32_t ans = 0;
  ans += ((uint16_t)Wire.read()) << 8;
  ans += Wire.read();
  return ans;
}

float BaroGetAlt(void)
{
  return alt;
}

float BaroGetVelo(void)
{
  return velo; 
}
  
