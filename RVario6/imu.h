#ifndef IMU_H_
#define IMU_H_

#include "Arduino.h"
#include "quaternionFilters.h"
#include "MPU9250.h"

void IMUInit(void);
void IMUPoll(void);
float IMUPitch(void);
float IMURoll(void);
void IMUPrint(void);
void printFloat(float f);
float IMUGetG(void);

#endif // _IMU_H_


