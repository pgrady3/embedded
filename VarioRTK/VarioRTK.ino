//#include <i2c_t3.h>
#include <Wire.h>
#include "SparkFun_Ublox_Arduino_Library.h"
#include "GPS.h"
#include "LiquidCrystal_I2C.h"

#define LED1 7
#define LED2 8

#define SD_CS 6

#include <SD.h>

volatile uint32_t loopTime = 0;
uint32_t sdOk = 0;
uint32_t statusReg = 0;

File myFile;
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  //Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.begin();
  Serial.begin(115200);
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  myFile = SD.open("data.txt", FILE_WRITE);

  delay(500);
  Serial.println("Trying print");
  lcd.begin();
  lcd.backlight();
  lcd.print("Hello, world!");

  GPSInit();
}

void loop() {
  uint32_t curTime = millis();

  GPSPoll();
  
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;
      
  digitalWrite(LED2, !digitalRead(LED2));
  loopTime = curTime;
  
  writeToBtSd();
}

void writeToBtSd() {
  String outputStr = String(GPSLat)     + "\t" + String(GPSLon)      + "\t" + String(GPSAlt) + "\t" + 
                     String(GPSFixType) + "\t" + String(GPSVelN)     + "\t" + String(GPSVelE) + "\t" + 
                     String(GPSVelD)    + "\t" + String(GPSHeading)  + "\t" + String(GPSVAcc)  + "\t" +
                     String(GPSHAcc)  + "\t" + String(millis());
  
  Serial.println(outputStr);//usb
  
  int sdWritten = myFile.println(outputStr);
  myFile.flush();
  sdOk = sdWritten;
}
