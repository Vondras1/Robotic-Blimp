#include "GPS.h"

TinyGPS GPS;

float latitude, longitude, speed, altitudeGPS;
unsigned long fix_age, date, tm;

/**
 * This thread handles communication with the GPS module and updates global variables with the current GPS data.
 * It runs continuously, retrieving new data at regular intervals.
 */
void thread_GPS(void){
  while(1){
    // Check for new data every 1000 milliseconds
    bool newdata = smartdelay(1000, GPS);
    if (newdata) {
      GPSData gpsdata = decodeGPS(GPS);
      latitude = gpsdata.flat;
      longitude = gpsdata.flon;
      altitudeGPS = gpsdata.altitude;
      fix_age = gpsdata.age;
      speed = gpsdata.ss;
      tm = gpsdata.time;
      date = gpsdata.date;
    }
    threads.delay(500);
  }
}

/**
 * Implements a delay while continuously checking for and processing new GPS data.
 * This allows for real-time GPS data processing without blocking the main thread.
 *
 * @param ms The amount of time to delay in milliseconds.
 * @param GPS The TinyGPS object used for processing GPS data.
 * @return true if new GPS data was received and processed during the delay; false otherwise.
 */
bool smartdelay(unsigned long ms, TinyGPS &GPS){
  unsigned long start = millis();
  bool newdata = false;
  do 
  {
    while (GPS_SERIAL.available()){
      GPS.encode(GPS_SERIAL.read());
      newdata = true;
    }
  } while (millis() - start < ms);
  return newdata;
}

/**
 * Decodes data received from the GPS module using the TinyGPS library.
 *
 * @param GPS The TinyGPS object used for processing GPS data.
 * @return A GPSData structure containing the decoded data: latitude, longitude, speed, altitude, date, and time.
 */
GPSData decodeGPS(TinyGPS &GPS){ 
  GPSData gpsdata;

  GPS.f_get_position(&gpsdata.flat, &gpsdata.flon, &gpsdata.age);
  //GPS.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  GPS.get_datetime(&gpsdata.date, &gpsdata.time, &gpsdata.age);

  gpsdata.ss = GPS.f_speed_kmph();

  gpsdata.altitude = GPS.f_altitude();

  return gpsdata;
}