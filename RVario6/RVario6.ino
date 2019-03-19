#include "imu.h"
#include "Adafruit_BMP280.h"

#define LED_G 7   // PE6
#define LED_B A5  //PF0
#define BMP_CS A4 //PF1

Adafruit_BMP280 bmp(BMP_CS);

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_B, HIGH);

  IMUInit();

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

uint32_t baroTime = 0;

void loop()
{
  uint32_t curTime = millis();

  if(curTime > baroTime + 100)
  {
    baroTime = curTime;
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.print(" ");
    Serial.println(IMUPitch());
  }
  
  IMUPoll();
}

