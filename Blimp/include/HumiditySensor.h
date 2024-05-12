#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include "GeneralLib.h"
#include <SimpleDHT.h>

#define DHTPIN 6     // Digital pin connected to the DHT sensor

void thread_DHT22(void);

#endif
