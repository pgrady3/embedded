#include "SparkFun_Ublox_Arduino_Library.h"
SFE_UBLOX_GPS myGPS;

#define GPS_SPS 8  //8hz results in fastest updates. I think 19200 baud rate is clogging up
#define GPS_DERIV_LEN 8 //about one second of delay

uint32_t GPSLastUpdate = 0;
uint32_t GPSFirstUpdate = 0;
uint32_t GPSCount = 0;
uint32_t GPSFreq = 0;
int32_t GPSLat;
int32_t GPSLon;
float GPSAlt;
int32_t GPSFixType;
float GPSVelN, GPSVelE, GPSVelD;
float GPSHeading;
int32_t GPSVAcc;
int32_t GPSHAcc;
float GPSTEVario;
float GPSTEHis[GPS_DERIV_LEN];
uint32_t GPSTEPos = 0;

void GPSInit() {
  Serial1.begin(19200); //Ublox RTK
  delay(500);

  while(myGPS.begin(Serial1) == false)
    delay(10);
  
  while(myGPS.setAutoPVT(true) != true) //Tell the GPS to "send" each solution
    delay(10);

  while(myGPS.setNavigationFrequency(GPS_SPS) != true)
    delay(10);

  delay(500);

  GPSFirstUpdate = millis();
}

void GPSPoll() {
  if (!myGPS.getPVT())
    return;
    
  GPSLat = myGPS.getLatitude();
  GPSLon = myGPS.getLongitude();
  GPSAlt = myGPS.getAltitude() / 1000.0;
  GPSFixType = myGPS.getFixType() + (myGPS.getCarrierSolutionType() * 10);//1x for floatRTK, 2x for fixedRTK
  GPSVelN = myGPS.velN / 1000.0;
  GPSVelE = myGPS.velE / 1000.0;
  GPSVelD = myGPS.velD / 1000.0;
  GPSHeading = myGPS.headingOfMotion / 100000.0;
  GPSVAcc = myGPS.vAcc;
  GPSHAcc = myGPS.hAcc;

  float totalvsq = GPSVelN * GPSVelN + GPSVelE * GPSVelE + GPSVelD * GPSVelD;
  float TE = 0.5 * totalvsq + 9.801 * GPSAlt;
  
  
  GPSTEVario = (TE - GPSTEHis[GPSTEPos]) * GPS_SPS / GPS_DERIV_LEN / 9.801;
  GPSTEHis[GPSTEPos++] = TE;
  GPSTEPos %= GPS_DERIV_LEN;
    
  GPSLastUpdate = millis();

  GPSFreq = (GPSLastUpdate - GPSFirstUpdate) / GPSCount++;
}
