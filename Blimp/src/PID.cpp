/* While writing this PID controller, my work was more than inspired by PID controller created by Brett Beauregard (https://github.com/br3ttb/Arduino-PID-Library).*/

#include "PID.h"

// Global variable declarations
double lastError;
float kp, ki, kd, N; // Proportional, Integral, and Derivative gains and smoothing factor
float PrecalculatedForD1, PrecalculatedForD2;
float SampleTimeInSec;
float filteredD = 0; 
float clasicD = 0;
float UpperLimit, LowerLimit; // Output limits
float IntegralPart;

/**
 * Sets the PID parameters and initializes some state variables.
 *
 * @param Kp Proportional gain.
 * @param Ki Integral gain.
 * @param Kd Derivative gain.
 * @param n Derivative filter smoothing factor.
 * @param SampleTime Time between PID calculations in milliseconds.
 */
void setParametres(float Kp, float Ki, float Kd, float n, unsigned int SampleTime){
  if(SampleTime > 0) SampleTime = SampleTime;
  else SampleTime = 200; // Default to 200ms if no sample time is specified

  SampleTimeInSec = ((float)SampleTime)/1000;

  kp = Kp;
  ki = Ki * SampleTimeInSec; // Integral gain adjusted for sampling time
  kd = Kd / SampleTimeInSec; // Derivative gain adjusted for sampling time
  N = n; 

  PrecalculatedForD1 = (kd) * ((N*SampleTimeInSec) / (1+N*SampleTimeInSec));
  PrecalculatedForD2 = (1 / (1+N*SampleTimeInSec));
  
  lastError = 0;
}

/**
 * Sets the upper and lower output limits for the PID controller.
 *
 * @param Max The maximum limit for the PID output.
 * @param Min The minimum limit for the PID output.
 */
void setLimits(float Max, float Min){
  if (Min < Max){
    UpperLimit = Max;
    LowerLimit = Min;
  }
  else { // Set no limits
    UpperLimit = INFINITY;
    LowerLimit = -INFINITY;
  }
}

/**
 * Calculates the PID output based on the current and required values.
 * This function has to be called at fixed intervals which are given by SampleTime (lastTime - now = SampleTime)
 * 
 * @param Output Reference to store the calculated output.
 * @param CurrentValue The current value from the sensor.
 * @param RequiredValue The desired setpoint value.
 */
void CalculateOutput(float &Output, double CurrentValue, double RequiredValue){
  //Calculate all the working error variables
  float error = RequiredValue - CurrentValue;
  double dErr = error - lastError;
  IntegralPart += error * ki;

  // Apply limits to integral component
  checkLimits(IntegralPart);

  //Calculate PID Output
  //filteredD = PrecalculatedForD1 * dErr + PrecalculatedForD2 * filteredD;
  clasicD = kd * dErr;
  Output = kp * error + IntegralPart + clasicD;

  // Apply limits to output
  checkLimits(Output);

  //Remember last error for next time
  lastError = error;
}

/**
 * Checks and enforces the previously set limits on a given value.
 *
 * @param val Reference to the value that needs to be limited.
 */
void checkLimits(float &val){
  if(val > UpperLimit) val = UpperLimit;
  else if(val < LowerLimit) val = LowerLimit;
}

/**
 * Initializes the PID controller, resetting all integral and state variables.
 * This function should be called to reset the PID controller to a known initial state before starting a new process or after any significant changes in setpoint.
 */
void initialization(void){
    lastError = 0;
    IntegralPart = 0;

}
