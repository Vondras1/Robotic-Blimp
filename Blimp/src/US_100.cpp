#include "US_100.h"

int ultrasonic_distance = 0;
float USValidityRate = 0.5;
const unsigned int BUFFER_SIZE = 40;
bool US_buffer[BUFFER_SIZE];

/**
 * Initialize the US_buffer
*/
void initialize_buffer(const unsigned int length){ // length has to be even number
    for(unsigned int i = 0; i < length/2; i++) {
        US_buffer[i] = 0;
    }
    for(unsigned int j = 10; j < length; j++) {
        US_buffer[j] = 1;
    }
}

/**
 * Computes the validity rate of the buffer 'US_buffer' after inserting a new boolean value.
*/
float ValidityRate(bool new_value){
    float cnt = 0;
    for(unsigned int i = 1; i < BUFFER_SIZE; i++){
        US_buffer[i-1] = US_buffer[i];
        cnt += US_buffer[i-1];
    }
    US_buffer[BUFFER_SIZE-1] = new_value;
    cnt += US_buffer[BUFFER_SIZE-1];
    float rate = (cnt/BUFFER_SIZE);
    return rate;
}

/**
 * An infinite loop for retrieving and processing data from the sonar sensor. 
 * This function is intended to run as a separate thread.
*/
void thread_ultrasonic(void) {
    unsigned int HighByte = 0;
    unsigned int LowByte  = 0;
    int current_dist = 0;
    int old_avg_distance = 0;
    initialize_buffer(BUFFER_SIZE);
    while(1) {
        SONAR_SERIAL.flush();
        SONAR_SERIAL.write(0X55); // distance measurement request
        unsigned long start = millis();
    
        // Waits for data from the sensor, but a maximum of 500 milliseconds
        while (not(SONAR_SERIAL.available() >= 2) and (millis() - start < 500)) {
            threads.delay(10);
        }

        if (SONAR_SERIAL.available() >= 2) {
            HighByte = SONAR_SERIAL.read();
            LowByte  = SONAR_SERIAL.read();
            current_dist = ReadSerialInput(HighByte, LowByte);
        } else {
            current_dist = INVALID_VALUE;
        }

        old_avg_distance = distance_EMA(current_dist, old_avg_distance);
        ultrasonic_distance = old_avg_distance;
        USValidityRate = ValidityRate((old_avg_distance != INVALID_VALUE));
        threads.delay(10);
    }
}


/**
 * Calculates the distance using Exponential Moving Average (EMA).
 * This function assumes that the height cannot be negative and 'INVALID_VALUE' is always negative.
 *
 * @param current_dist The current distance measurement.
 * @param old_avg_distance The previous average distance.
 * @param a The weight for the current distance in the EMA calculation.
 * @param b The weight for the previous average distance in the EMA calculation.
 * @return The new average distance or INVALID_VALUE if the current distance is invalid.
 */
int distance_EMA(int current_dist, int old_avg_distance, float a = 0.4, float b = 0.60){
    if (current_dist < 0){
        // If the current distance is invalid, return the invalid value
        return INVALID_VALUE;
    }else{
        if(old_avg_distance <= 0){
            // If the old average distance is not valid, use the current distance as the new average
            return current_dist;
        }
        // Otherwise, calculate the new average using the EMA formula
        return (int)(a*current_dist + b*old_avg_distance);
  }
}


/**
 * Calculates the average distance from an array of distance values.
 * If no valid value is found, it returns INVALID_VALUE.
 *
 * @param ArrayOfValues An array of unsigned integers representing distance measurements.
 * @param size The number of elements in the array.
 * @return The average distance as a double. Returns INVALID_VALUE if no valid measurement is found.
 */
double get_average_distance(unsigned int ArrayOfValues[], int size){
    unsigned int sum = 0;
    unsigned int cnt = 0;
    for(int i = 0; i < size; i++){
        if (ArrayOfValues[i] != INVALID_VALUE){
            sum += ArrayOfValues[i];
            cnt += 1;
        }
    }
    if (cnt == 0) return INVALID_VALUE;
    else return (double)(sum/cnt);
}

/**
 * Reads the distance measured by the ultrasonic sensor.
 * It combines two bytes into a single integer to represent the measured distance.
 * If the measured distance is outside the valid range, it returns INVALID_VALUE.
 *
 * @param MSB The Most Significant Byte of the distance measurement.
 * @param LSB The Least Significant Byte of the distance measurement.
 * @return The distance in millimeters if within valid range, or INVALID_VALUE if not conclusive.
 */
int ReadSerialInput(unsigned int MSB, unsigned int LSB){
    int Dist_mm  = (MSB << 8) | LSB;   // Shifts the upper byte eight positions to the left to form one number from two messages
    if ((Dist_mm > 0) && (Dist_mm < 3500)){  ///TODO - 3500
        return  Dist_mm;
    }
    else return INVALID_VALUE;
}
