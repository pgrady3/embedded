#ifndef BARO_H
#define BARO_H

void BaroInit(void);
void BaroPoll(void);
float BaroGetAlt(void);
float BaroGetVelo(void);

#endif
