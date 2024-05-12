#include "PressureSensor.h"
#include "US_100.h"
#include <math.h>

const float SEA_LEVEL_PRESSURE = 102630; // (Pa)
const float GIVEN_TEMPERATURE = 273.15 + 15; // (K)

// It is also possible to use the automatic mode without manually entering the temperature 
// to determine the altitude, but it is not as accurate. For Automatic set AUTOMATIC to true
bool AUTOMATIC = false;

float pressure = 0;
float altitude = 0;
float temperature_MPL3115A2 = 0;

MPL3115A2 MPL;

/**
 * This thread continuously reads temperature and pressure data from the MPL3115A2 sensor.
 * It applies an Exponential Moving Average (EMA) low pass filter to smooth the data.
 * Also, it calculates the altitude based on the pressure and temperature readings.
 */
void thread_MPL3115A2(void){
  // auxiliary variables
  float avg_temperature = 0;
  float old_avg_temperature = 0;
  float avg_pressure = 0;
  float old_avg_pressure = 0;
  int cnt = 0;
  while(1){
    // Read the current values
    float temp = MPL.readTemp();
    float press = MPL.readPressure();

    // Skip initial sensor readings
    if (cnt < 7) {
      cnt += 1;
      continue;
    }

    // Validate and update temperature average
    if (temperature_value_is_valid(temp)) {
      old_avg_temperature = (avg_temperature == 0) ? temp : avg_temperature;
      avg_temperature = temperature_EMA(temp, old_avg_temperature);
      temperature_MPL3115A2 = avg_temperature;
    }
    // Check if the pressure value is valid
    if (preasure_value_is_valid(press)){
      // Update the average pressure
      old_avg_pressure = (avg_pressure == 0) ? press : avg_pressure;
      avg_pressure = pressure_EMA(press, old_avg_pressure);
      // Calculate altitude
      if (AUTOMATIC){
        altitude = calculate_altitude(avg_pressure, SEA_LEVEL_PRESSURE, temperature_MPL3115A2 + 273.15); // This 
      }else{
        altitude = calculate_altitude(avg_pressure, SEA_LEVEL_PRESSURE, GIVEN_TEMPERATURE);
      }
      //Serial.print("Altitude: "), Serial.print(altitude), Serial.print(", Pressure: "), Serial.print(pressure);
      //Serial.print(", Actual pressure: "), Serial.println(press);
      pressure = avg_pressure;
    }
    // Delay the thread for 512 milliseconds before the next iteration
    threads.delay(256);
  }
}

/**
 * Calculates altitude from the given pressure readings using the barometric formula.
 * This function requires the current pressure, sea level pressure, and temperature in Kelvin.
 *
 * @param P Current pressure reading from the sensor.
 * @param sea_pressure Sea level pressure, used as a reference for calculating altitude.
 * @param T_kelvin Temperature in Kelvin. If not provided, defaults to 288 Kelvin.
 * @return Calculated altitude (in m) based on the current pressure and temperature.
 */
float calculate_altitude(float P, float sea_pressure, float T_kelvin = 293){
  float R = 8.3143; // universal gas constant (J/(mol*K))
  float g = 9.8066; // gravitational acceleration constant (m/s^2)
  float M = 0.0289; // molar mass of Earth’s air (kg/mol)
  float Altitude = -((R * T_kelvin)/(M*g)) * log(P/sea_pressure); // Barometric formula 
  //float alt = -(SEA_LEVEL_PRESSURE/(1.2932 * g) * log(pressure/SEA_LEVEL_PRESSURE)); // Alternative, less accurate calculation option

  return Altitude;
}

/**
 * Calculates the Exponential Moving Average (EMA) for pressure readings.
 * EMA is used to smooth out the data, reducing the impact of short-term fluctuations.
 *
 * @param measured_pressure The current pressure reading from the sensor.
 * @param old_average The previous EMA value of the pressure.
 * @param a The weighting given to the current measurement (default: 0.35).
 * @param b The weighting given to the previous average (default: 0.65).
 *      Note: a + b should equal 1 to maintain a consistent scale.
 * @return The new EMA value for the pressure.
 */
float pressure_EMA(float meassured_pressure, float old_average, float a = 0.15, float b = 0.85){
  float new_average = (a*meassured_pressure + b*old_average);
  return new_average;
}

/**
 * Calculates the Exponential Moving Average (EMA) for temperature readings.
 * EMA is utilized to smooth out temperature data, reducing the impact of short-term variations.
 *
 * @param measured_temperature The current temperature reading from the sensor.
 * @param old_average The previous EMA value of the temperature.
 * @param a The weighting given to the current measurement (default: 0.25).
 * @param b The weighting given to the previous average (default: 0.75).
 *      Note: a + b should equal 1 to maintain a consistent scale.
 * @return The new EMA value for the temperature.
 */
float temperature_EMA(float meassured_temperature, float old_average, float a = 0.2, float b = 0.8){
  float new_average = (a*meassured_temperature + b*old_average);
  return new_average;
}

bool preasure_value_is_valid(float P){
  if(P > 0 and !isnan(P)) return true;
  else return false;
}

bool temperature_value_is_valid(float t){
  if(!isnan(t)) return true;
  else return false;
}
