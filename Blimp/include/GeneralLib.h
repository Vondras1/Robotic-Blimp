#ifndef GeneralLib_h
#define GeneralLib_h

#include <Arduino.h>
#include <SPI.h>
#include <TeensyThreads.h>
#include <Wire.h>

//MPL3115A2
extern float pressure;
extern float altitude;
extern float temperature_MPL3115A2;

//DHT_22
extern float humidity;
extern float temperature_DHT22;

//GPS modul
extern float latitude, longitude, speed, altitudeGPS;
extern unsigned long fix_age, date, tm;

//communication
extern bool LAND, FLY_FORWARD;
extern float REQ_HEIGHT; // Target height in m
extern float C_R_H; // Current target height in m // finally it should have same value like REQ_HEIGHT
extern float CURRENT_HEIGHT;
extern int ANGLE;
extern int POWER_OF_STEERING_MOTOR;
extern unsigned int LAST_COUNTER;
extern unsigned char RECEIVED_ID;
extern int RECEIVED_REQ_HEIGHT; // Required height sent from controller
extern bool VALID_MSG;
extern bool NEW_MSG;
extern long lastSendTime;        // last send time
extern long lastReceivedTime;        // last received time

// US-100
extern int ultrasonic_distance;
extern bool ultrasonicDistanceIsValid;
extern float USValidityRate;

// Motor controll
extern unsigned int SampleTime;
extern bool DoNotMove;
extern unsigned int POWER;

#endif