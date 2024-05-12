#include "HumiditySensor.h"
#include "US_100.h"

float humidity = 0;
float temperature_DHT22 = 0;

SimpleDHT22 dht22(DHTPIN);

/**
 * This thread is dedicated to measuring humidity and temperature using the DHT22 sensor.
 * It runs continuously in a loop, updating the humidity and temperature readings.
 */
void thread_DHT22(void){
  while(1){
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(&temperature_DHT22, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.println("Read DHT22 failed.");
      temperature_DHT22 = INVALID_VALUE;
      humidity = INVALID_VALUE;
    }

    threads.delay(2500);
  }
}

/*
WIRING - DHT22 
- Connect pin 1 (on the left) of the sensor to +3.3V |u puntíku|
- Connect pin 2 of the sensor to DHTPIN (22)
- Do not connect pin 3
- Connect pin 4 (on the right) of the sensor to GROUND
- Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor - It's pull-up resistor
*/