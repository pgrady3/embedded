#include <SparkFun_Ublox_Arduino_Library.h>
SFE_UBLOX_GPS myGPS;

uint32_t GPSLastUpdate = 0;
int32_t GPSLat;
int32_t GPSLon;
int32_t GPSAlt;
int32_t GPSFixType;
 

void GPSInit() {
  Serial1.begin(19200); //Ublox RTK
  delay(500);

  while(myGPS.begin(Serial1) == false)
    delay(10);
  
  while(myGPS.setAutoPVT(true) != true) //Tell the GPS to "send" each solution
    delay(10);

  while(myGPS.setNavigationFrequency(8) != true) //8hz results in fastest updates. I think 19200 baud rate is clogging up
    delay(10);

  delay(500);
}

void GPSPoll() {
  if (!myGPS.getPVT())
    return;
    
  long latitude = myGPS.getLatitude();
  Serial.print(F("Lat: "));
  Serial.print(latitude);

  long longitude = myGPS.getLongitude();
  Serial.print(F(" Long: "));
  Serial.print(longitude);
  Serial.print(F(" (degrees * 10^-7)"));

  long altitude = myGPS.getAltitude();
  Serial.print(F(" Alt: "));
  Serial.print(altitude);
  Serial.print(F(" (mm)"));

  byte SIV = myGPS.getSIV();
  Serial.print(F(" SIV: "));
  Serial.print(SIV);

  Serial.println();

  GPSLastUpdate = millis();

}
