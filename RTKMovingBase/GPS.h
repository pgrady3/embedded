#include "SparkFun_Ublox_Arduino_Library.h"
SFE_UBLOX_GPS myGPS;


int32_t GPSLat;
int32_t GPSLon;
float GPSAlt;
int32_t GPSFixType;
float GPSVelN, GPSVelE, GPSVelD;
float GPSRelN, GPSRelE, GPSRelD;

void GPSInit() {
  Serial1.begin(19200); //Ublox RTK
  delay(500);

  while(myGPS.begin(Serial1) == false)
    delay(10);

  delay(500);
}

void GPSPoll() {
  myGPS.getPVT();
  myGPS.getRELPOSNED();
    
  GPSLat = myGPS.getLatitude();
  GPSLon = myGPS.getLongitude();
  GPSAlt = myGPS.getAltitude() / 1000.0;
  GPSFixType = myGPS.getFixType() + (myGPS.getCarrierSolutionType() * 10);//1x for floatRTK, 2x for fixedRTK
  GPSVelN = myGPS.velN / 1000.0;
  GPSVelE = myGPS.velE / 1000.0;
  GPSVelD = myGPS.velD / 1000.0;
  GPSRelN = myGPS.relPosInfo.relPosN;
  GPSRelE = myGPS.relPosInfo.relPosE;
  GPSRelD = myGPS.relPosInfo.relPosD;
  

}
