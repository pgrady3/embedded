#include "imu.h"
#include "Adafruit_BMP280.h"
#include "ms5611.h"

#define LED_G 7         // PE6
#define LED_B A5        //PF0
#define BMP_CS A4       //PF1
#define MS5611_1_CS A2    // PF5
#define MS5611_2_CS A1    // PF6 
#define FLASH_CS A0     // PF7
#define MS5525_1_CS A8    // PB5
#define MS5525_2_CS 13    // PC7


Adafruit_BMP280 bmp(BMP_CS);
MS5611 ms5611_1;
MS5611 ms5611_2;

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_B, HIGH);
  pinMode(MS5525_2_CS, OUTPUT);
  digitalWrite(MS5525_2_CS, HIGH);
  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH);
  pinMode(MS5525_1_CS, OUTPUT);
  digitalWrite(MS5525_1_CS, HIGH);

  IMUInit();
  ms5611_1.init(MS5611_1_CS);
  ms5611_2.init(MS5611_2_CS);

  if (!bmp.begin())
    digitalWrite(LED_G, HIGH);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

uint32_t baroTime = 0;
float off1 = 0;
float off2 = 0;
float off3 = 0;

void loop()
{
  uint32_t curTime = millis();

  /*if(curTime > baroTime + 20)
  {
    digitalWrite(LED_B, !digitalRead(LED_B));
    baroTime = curTime;
    
    float bmpAlt = bmp.readAltitude(1013.25);
    float ms1Alt = ms5611_1.getAlt();
    float ms2Alt = ms5611_2.getAlt();

    if(millis() < 10000)
    {
      off1 = bmpAlt;
      off2 = ms1Alt;
      off3 = ms2Alt;
    }

    IMUPrint();
    //printFloat((bmpAlt - off1) * 100);
    printFloat((ms1Alt - off2));
    //printFloat((ms2Alt - off3) * 100);
    //Serial.print(millis());
    Serial.println();
    
  }*/
  
  //ms5611_1.poll();
  //ms5611_2.poll();
  IMUPoll();
  IMUPrint();
}

