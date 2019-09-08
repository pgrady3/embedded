//#include <i2c_t3.h>
#include <Wire.h>
#include "SparkFun_Ublox_Arduino_Library.h"
#include "GPS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED1 7
#define LED2 8

#define SD_CS 6

#include <SD.h>

volatile uint32_t loopTime = 0;
uint32_t sdOk = 0;
uint32_t statusReg = 0;

File myFile;

#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() {
  //Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.begin();
  Wire.setClock(100000);
  
  Serial.begin(115200);
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  myFile = SD.open("data.txt", FILE_WRITE);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  while(1)
  {
    display.clearDisplay();
    
    // display a pixel in each corner of the screen
    display.drawPixel(0, 0, WHITE);
    display.drawPixel(127, 0, WHITE);
    display.drawPixel(0, 63, WHITE);
    display.drawPixel(127, 63, WHITE);
    
    // display a line of text
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(27,30);
    display.print("Hello, world!");
    
    // update display with all of the above graphics
    display.display();

    delay(5000);
  }

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
