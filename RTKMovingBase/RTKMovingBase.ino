#include <Wire.h>
#include "SparkFun_Ublox_Arduino_Library.h"
#include "GPS.h"

#define LED1 7
#define LED2 8

#define SD_CS 6

#include <SD.h>

volatile uint32_t loopTime = 0;
uint32_t sdOk = 0;
uint32_t statusReg = 0;

File myFile;

void setup() {
  //Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.begin();
  Wire.setClock(400000);
  
  Serial.begin(230400);
  
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  myFile = SD.open("data.txt", FILE_WRITE);
  
  GPSInit();
}

void loop() {
  uint32_t curTime = millis();

  GPSPoll();
  delay(2000);

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
                     String(GPSFixType) + "\t" + String(GPSVelN)     + "\t" + String(GPSVelE) + "\t" + String(GPSVelD) + "\t" + 
                     String(GPSRelN)     + "\t" + String(GPSRelE) + "\t" + String(GPSRelD) + "\t" + 
                     String(millis());

  //String outputStr = String(BaroGetVelo());
  
  Serial.println(outputStr);//usb
  
  int sdWritten = myFile.println(outputStr);
  myFile.flush();
  sdOk = sdWritten;
}
