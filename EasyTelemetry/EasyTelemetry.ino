#include <i2c_t3.h>
#include <SD.h>
#include "Adafruit_GPS.h"
#include "INA.h"

#define LED1 21
#define LED2 22

#define HALL 4
#define SD_CS 8

#define WHEEL_CIRC 1.492
#define WHEEL_TICKS 8
#define TICK_DIST (WHEEL_CIRC / WHEEL_TICKS)

#define TARGET_CURRENT 5

volatile uint32_t tickTimes[WHEEL_TICKS];
volatile uint32_t tickPos;

volatile uint32_t loopTime = 0;
volatile uint32_t lastHallPulse = 0;
volatile uint32_t lastInaMeasurement = 0;
volatile uint32_t countIntervals = 0;
volatile int32_t avgdT = 1000000;
volatile uint32_t distTicks = 0;

double energyUsed = 0.0;
double distance = 0.0;
double currentSpeed = 0.0;
double InaVoltage = 0.0;
double InaCurrent = 0.0;
double InaPower = 0;
double batteryVoltage = 0.0;

File myFile;
Adafruit_GPS GPS(&Serial1);

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  INAinit();

  Serial.begin(115200);
  
  SD.begin(SD_CS);
  delay(100);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pinMode(HALL, INPUT_PULLUP);
  attachInterrupt(HALL, countHallPulse, FALLING);

  myFile = SD.open("data.txt", FILE_WRITE);
  
  GPSInit();
}


void loop() {  
  GPSPoll();//must be called rapidly
  
  uint32_t curTime = millis();
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;

  digitalWrite(LED2, !digitalRead(LED2));
  loopTime = curTime;
  
  updateINA();
  updateSpeed();

  writeToBtSd();
}

void updateINA()
{
  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  InaPower = InaVoltage * InaCurrent;
  
  double currentInaTime = millis();
  energyUsed += InaPower * (currentInaTime - lastInaMeasurement) / 1000;
  lastInaMeasurement = currentInaTime;  
}

void updateSpeed()
{
  currentSpeed = 1000000.0 / avgdT * WHEEL_CIRC; 
  if(micros() - lastHallPulse > 2000000)
    currentSpeed = 0;
  
  distance = distTicks * TICK_DIST;
}

void countHallPulse() {
  uint32_t current = micros();

  uint32_t prevTime = tickTimes[tickPos];// time 1 rev ago. fixed 7/5/2018 :(

  tickTimes[tickPos++] = current;
  tickPos %= WHEEL_TICKS;

  avgdT = current - prevTime;

  distTicks++;
  
  lastHallPulse = current;

  digitalWrite(LED1, (distTicks) & 1);
}

void GPSInit()
{
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_BAUD_57600);
  delay(500);
  Serial1.end();
  
  delay(500);
  GPS.begin(57600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);   // 10 Hz update rate
}

void GPSPoll()
{
  while(GPS.read());
  
  if (GPS.newNMEAreceived())
    GPS.parse(GPS.lastNMEA());
}

void writeToBtSd() {
  String outputStr = String(InaVoltage, 3) + " " + String(InaCurrent, 3) + " " + String(InaPower) + " "+ String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(millis()) + " " + String(GPS.latitudeDegrees, 7) + 
                     " " + String(GPS.longitudeDegrees, 7);
  
  
  Serial.println(outputStr);//usb  
  GPSPoll();//super hacky bc short GPS buffer
  
  uint32_t startTime = micros();
  myFile.println(outputStr);
  myFile.flush();
}
