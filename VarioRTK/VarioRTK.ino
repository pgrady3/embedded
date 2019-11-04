#include <Wire.h>
#include "SparkFun_Ublox_Arduino_Library.h"
#include "GPS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Baro.h"
#include "MPU9250.h"

#define LED1 7
#define LED2 8

#define SD_CS 6

#include <SD.h>

volatile uint32_t loopTime = 0;
uint32_t sdOk = 0;
uint32_t statusReg = 0;

File myFile;

#define OLED_ADDR   0x3C
//Adafruit_SSD1306 display(-1);

void setup() {
  //Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.begin();
  Wire.setClock(400000);
  
  Serial.begin(230400);
  Serial2.begin(115200);//bluetooth
  
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  myFile = SD.open("data.txt", FILE_WRITE);
  
  //display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  BaroInit();
  IMUInit();
  GPSInit();
}

void loop() {
  uint32_t curTime = millis();

  GPSPoll();
  IMUPoll();
  BaroPoll();
  
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;

  loopTime = curTime;
  digitalWrite(LED2, !digitalRead(LED2));
  
  writeToBtSd();
  //drawLCD();
}

/*void drawLCD() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(BaroGetAlt());

  display.setCursor(0,10);
  display.print(GPSAlt);

  display.setCursor(0,20);
  display.print(GPSFixType);
  display.print(" sd ");
  display.print(sdOk);

  //display.setCursor(0,30);
  //display.print(IMUGetG());
  
  display.display();
}*/

void writeToBtSd() {
  String outputStr = String(GPSLat)     + "\t" + String(GPSLon)      + "\t" + String(GPSAlt) + "\t" + 
                     String(GPSFixType) + "\t" + String(GPSTEVario)     + "\t" + String(GPSVelN)     + "\t" + String(GPSVelE) + "\t" + 
                     String(GPSVelD)    + "\t" + String(GPSHeading)  + "\t" + String(GPSVAcc)  + "\t" +
                     String(GPSHAcc)  + "\t" + String(BaroGetAlt())  + "\t" + String(BaroGetVelo())  + "\t" + 
                     String(ax)    + "\t" + String(ay)  + "\t" + String(az)  + "\t" +
                     String(roll)    + "\t" + String(pitch)  + "\t" + String(yaw)  + "\t" +
                     String(millis());

  //String outputStr = String(BaroGetVelo());
  
  Serial.println(outputStr);//usb
  Serial2.println(outputStr);
  
  int sdWritten = myFile.println(outputStr);
  myFile.flush();
  sdOk = sdWritten;
}
