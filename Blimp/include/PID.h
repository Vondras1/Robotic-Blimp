#ifndef PID_h
#define PID_h

#include <Arduino.h>
#include "GeneralLib.h"

// function declarations
void setParametres(float Kp, float Ki, float Kd, float n, unsigned SampleTime);
void setLimits(float Max, float Min);
void CalculateOutput(float &Output, double CurrentValue, double RequiredValue);
void checkLimits(float &val);
void initialization(void);

#endif