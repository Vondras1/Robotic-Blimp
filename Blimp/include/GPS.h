#ifndef GPS_H
#define GPS_H

#include "GeneralLib.h" // General library for global variables and libraries
#include <TinyGPS.h>

#define GPS_SERIAL Serial4

extern TinyGPS GPS;

// Structure to hold GPS data
struct GPSData {
    float flat; // latitude 
    float flon; // longitude
    float ss; // speed in km per hour
    float altitude; // height above sea level
    unsigned long date; // date [ddmmyy]
    unsigned long time; // time [hhmmsscc]
    unsigned long age; // time since the last GPS signal reception
};

// Function declarations
void thread_GPS(void);
bool smartdelay(unsigned long ms, TinyGPS &GPS);
GPSData decodeGPS(TinyGPS &GPS);

#endif // GPS_H