#include "Potentiometer.h"
#include <math.h>

const unsigned int Potentiometer = A9;
float CENTER_value = 526;
volatile float ANGLE = 0;
float last_angle = 0;
bool READING = false;
float k = 3.91;
float q = -8.91;

/**
 * Check if the rotation angle of the encoder has changed.
 * 
 * @return Returns true if the value has changed, otherwise returns false.
*/
bool angle_change(void){
  // setting limits
  /*if (current > 130){
    current = 130;
  }else if (current < -130){
    current = -130;
  }*/
  float current = ANGLE;
  if (current > 90){
    current = 90;
  }else if (current < -90){
    current = -90;
  }

  if (abs(last_angle - current) >= 5 && !READING){
    last_angle = current;
    return true;
  }
  return false;
}

// Přednastaví 
void SetCentreValue(void){
  CENTER_value = get_voltage_value();
}

void thread_upgrade_angle(void){ // TODO - úprava časů
  float current = ANGLE;
  while(1){
    ANGLE = get_relative_angle(get_voltage_value(), CENTER_value);
    if (abs(ANGLE - current) >= 2){
      current = ANGLE;
      READING = true;
      threads.delay(15);
    }else {
      READING = false;
      threads.delay(100);
    }
  }
}

/**
 * Calculates the relative angle of the potentiometer.
 * It computes the angle relative to a given center value.
 *
 * @param voltage_value The current voltage reading from the potentiometer.
 * @param center_value The center (reference) value for calculating the relative angle.
 * @return The relative angle calculated from the voltage value.
 */
float get_relative_angle(float voltage_value, float center_value) {
  float center_angle = (center_value - q)/k;
  float curr_angle = (voltage_value - q)/k;
  return center_angle - curr_angle;
}

/**
 * Calculates the absolute angle of the potentiometer.
 * It uses a linear equation based on the voltage reading.
 *
 * @param voltage_value The voltage reading from the potentiometer.
 * @return The absolute angle calculated from the voltage value.
 */
float get_absolute_angle(float voltage_value) {
  return (voltage_value - q)/k;
}

/**
 * Centers the potentiometer reading.
 * It resets the CENTER_value to the current potentiometer reading.
 */
void center_it(void){
  CENTER_value = get_voltage_value();
}

/**
 * Measures and returns the most accurate voltage value from the potentiometer.
 * It uses a Z-score filtering method to find the value closest to the mean.
 *
 * @return The most accurately measured voltage value.
 */
float get_voltage_value(void) {
  unsigned int num = 5;
  int values[num];
  int sum = 0;

  for(unsigned int i = 0; i < num; i++){
    values[i] = analogRead(Potentiometer);
    sum += values[i];
  }
  
  float average_value = sum / num;

  int S_D_twice = 0;
  for(unsigned int i = 0; i < num; i++){
    S_D_twice += pow((values[i] - average_value), 2);
  }

  int standard_deviation = sqrt(S_D_twice/num);
  if (standard_deviation == 0){return average_value;} // If all measured values are the same, the function ends here.

  int most_accurate_value;
  float lowest_score = INFINITY;
  for(unsigned i = 0; i < num; i++){
    float score = (values[i]-average_value)/standard_deviation;

    if (lowest_score > score){most_accurate_value = values[i];}
  }

  return most_accurate_value;
}

