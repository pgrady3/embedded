#include "imu.h"

#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0

#define AHRS true         // Set to false for basic data read
#define SerialDebug false  // Set to true to get Serial output for debugging

MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);

float totalG = 0;

void IMUInit(void)
{
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.selfTest);
    
    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    
    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    
    if (d != 0x48)
    {
      // Communication failed, stop here
      Serial.println(F("Communication failed, abort!"));
      Serial.flush();
      abort();
    }

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.factoryMagCalibration);
    // Initialize device for active mode read of magnetometer
    
    if (SerialDebug)
    {
      //  Serial.println("Calibration values: ");
      Serial.print("X-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[0], 2);
      Serial.print("Y-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[1], 2);
      Serial.print("Z-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[2], 2);
    }

    // Get sensor resolutions, only need to do this once
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();

  } // if (c == 0x71)
  else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);

    // Communication failed, stop here
    Serial.println(F("Communication failed, abort!"));
    Serial.flush();
    abort();
  }
}

void IMUPoll(void)
{
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes - myIMU.accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes - myIMU.accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes - myIMU.accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
               * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
               * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
               * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];


    myIMU.updateTime();

    MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx * DEG_TO_RAD,
                         myIMU.gy * DEG_TO_RAD, myIMU.gz * DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);

    myIMU.yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ()
              * *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1)
              * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) - *(getQ()+3)
              * *(getQ()+3));
    myIMU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ()
              * *(getQ()+2)));
    myIMU.roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2)
              * *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1)
              * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) + *(getQ()+3)
              * *(getQ()+3));
    myIMU.pitch *= RAD_TO_DEG;
    myIMU.yaw   *= RAD_TO_DEG;
    myIMU.roll *= RAD_TO_DEG;

    totalG = myIMU.ax * myIMU.ax + myIMU.ay * myIMU.ay + myIMU.az * myIMU.az;
    totalG = sqrt(totalG);

    myIMU.count = millis();
    myIMU.sumCount = 0;
    myIMU.sum = 0;
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
}

float axOff = 0;
float xVelo = 0;
uint32_t lastTime = 0;

void IMUPrint(void)
{
  if(axOff == 0)//first run
  {
    axOff = myIMU.ax;
    lastTime = millis();
    return;
  }
  uint32_t curTime = millis();
  float dt = ((float)curTime - lastTime) / 1000;
  
  xVelo += (myIMU.ax - axOff) * 9.8 * dt;

  //printFloat((myIMU.ax - axOff) * 9.8);
  printFloat(totalG);
  //printFloat(myIMU.ax);
  //printFloat(myIMU.ay);
  //printFloat(myIMU.az);
  //Serial.println();

  lastTime = curTime;
}

void printFloat(float f)
{
  Serial.print(f, 3);
  Serial.print(" ");
}

float IMUPitch(void)
{
  return myIMU.pitch;
}

float IMURoll(void)
{
  return myIMU.roll;
}

float IMUGetG(void)
{
  return totalG;
}

