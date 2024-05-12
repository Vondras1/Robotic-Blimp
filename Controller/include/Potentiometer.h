#ifndef Potentiometer_h
#define Potentiometer_h

#include <Arduino.h>
#include <TeensyThreads.h>

extern const unsigned int Potentiometr;
extern float CENTER_value;
extern volatile float ANGLE;
extern float last_angle;
extern bool READING;
extern float k; //TODO
extern float q; //TODO

float get_voltage_value(void);
float get_absolute_angle(float voltage_value);
float get_relative_angle(float voltage_value, float center_value);
void center_it(void);
void SetCentreValue(void);
void thread_upgrade_angle(void);
bool angle_change(void);

#endif