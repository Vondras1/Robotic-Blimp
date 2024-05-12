#ifndef US_100_H
#define US_100_H

#include "GeneralLib.h"

#define SONAR_SERIAL Serial1
#define ARRAY_SIZE_ULTRASONIC 5
#define INVALID_VALUE -11111

void initialize_buffer(const unsigned int length);
float ValidityRate(bool last);
void thread_ultrasonic(void);
int ReadSerialInput(unsigned int MSB, unsigned int LSB);
double get_average_distance(unsigned int ArrayOfValues[], int size);
int distance_EMA(int current_dist, int old_avg_distance, float a = 0.40, float b = 0.60);

#endif // US_100_H