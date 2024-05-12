#ifndef PressureSensor_h
#define PressureSensor_h

#include "GeneralLib.h"
#include "SparkFunMPL3115A2.h"

extern MPL3115A2 MPL;

void thread_MPL3115A2(void);
float calculate_altitude(float P, float sea_pressure, float T_kelvin = 293);
float pressure_EMA(float meassured_pressure, float old_average, float a = 0.15, float b = 0.85);
float temperature_EMA(float meassured_temperature, float old_average, float a = 0.20, float b = 0.80);
bool preasure_value_is_valid(float P);
bool temperature_value_is_valid(float t);

#endif