/**
 * Project: Robotic Blimp
 * Author: Vojtěch Vondráček
 * Date: 12.5.2024
 * Description: Implementation of Robotic Blimp control program.
 * Note: Some comments were co-created by generative artificial intelligence.
 */

#include "GeneralLib.h"
#include "US_100.h"
#include "PressureSensor.h"
#include "HumiditySensor.h"
#include "GPS.h"
#include "SD_card.h"
#include "Communication.h"
#include <Servo.h>
#include "ControlMotors.h"

const unsigned int LED = 2;
bool LED_shine = false;

double initial_height = 0;
bool ultrasonicDistanceIsValid = false;
bool DoNotMove = true;
unsigned int EmergencyPeriod = 50000; 
unsigned long int lastLedTime = millis();
bool lastUltrasonicDistanceWasValid;
float old_height;
float lastSentTimeInterval;

//Motor controll constants
float Kp = 0.9;
float Ki = 0.08;
float Kd = 1.3;
float n = 30;
unsigned int SampleTime = 200;
float Upper = 3.5596;
float Lower = -1.9945;

double get_current_height(void);
void handle_validity_of_required_height(float old_height, bool lastUltrasonicDistanceWasValid);
void display_values(void);
void ControlSignalingDiode(void);

void setup() {
  Serial.begin(9600);
  Wire.begin();        // Join i2c bus
  GPS_SERIAL.begin(9600); // Serial port for GPS
  SONAR_SERIAL.begin(9600); // Serial port for Ultrasonic sensor

  //MPL3115A2
  MPL.begin(); // Get sensor online
  MPL.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  MPL.setOversampleRate(7); // Set Oversample to the recommended 128
  MPL.enableEventFlags(); // Enable all three pressure and temp event flags

  Serial.println("Airship initializating");
  // LoRa
  LoRa.setPins(csPin, resetPin, dio0); // set CS, reset, IRQ pin
  if (!LoRa.begin(433E6)) {             // initialize ratio at 433 MHz
    Serial.println("LoRa init failed. Check connections.");
    while (true);                       // if failed, do nothing
  }

  // must be done before ControlHeight threat is started
  InitializeESCs(); // takes 1.5 seconds
  setParametres(Kp, Ki, Kd, n, SampleTime);
  setLimits(Upper, Lower);
  
  InitializeSERVO();
  
  // Initialize led pin as output
  pinMode(LED, OUTPUT);
  // Set led pin to LOW
  digitalWrite(LED, LOW);

  //threads
  threads.addThread(thread_ultrasonic);
  threads.addThread(thread_DHT22);
  threads.addThread(thread_MPL3115A2);
  threads.addThread(thread_GPS);
  threads.addThread(ControlHeight);
  threads.addThread(ControlSteeringMotor);
  
  LoRa.onReceive(onReceive);
  LoRa.receive();

  Serial.println("Airship initialization done.");
}

void loop() {
  ControlServo(ANGLE);

  if(lastReceivedTime != 0 && abs(millis() - lastReceivedTime) > EmergencyPeriod){
    SetLAND();
  }

  lastUltrasonicDistanceWasValid = ultrasonicDistanceIsValid;
  old_height = CURRENT_HEIGHT;
  CURRENT_HEIGHT = get_current_height();
  handle_validity_of_required_height(old_height, lastUltrasonicDistanceWasValid);

  ControlSignalingDiode();

  if (NEW_MSG == true){
    process_command();
    NEW_MSG = false;
    LoRa.receive();
  }else if (abs(millis() - lastSendTime) > 700){
    send_measured_data();
    LoRa.receive();
  }
 
  threads.delay(100);
}

/**
 * Based on the measurements of the ultrasonic sensor and the pressure sensor, it determines the current height.
 * 
 * @return Current height [m]
*/
double get_current_height(void) {
  if (initial_height == 0) initial_height = altitude;
  if (ultrasonic_distance != INVALID_VALUE){
    ultrasonicDistanceIsValid = true;
    double ultrasonic_distance_in_m = ((double)ultrasonic_distance)/1000; //converting mm to m
    initial_height = altitude - ultrasonic_distance_in_m; // After each successful measurement of the ultrasonic sensor, the initial height is calibrated
    return ultrasonic_distance_in_m;
  }
  else{
    ultrasonicDistanceIsValid = false;
    return altitude - initial_height;
  }
}

/**
 * Adjusts the required height (REQ_HEIGHT) based on the last ultrasonic distance measurement.
 * This feature is used when transitioning from an invalid to a valid ultrasonic distance measurement that 
 * occurs as the airship approaches the ground.
 * It recalculates REQ_HEIGHT to ensure that it remains a feasible target height above ground level, adjusting
 * it based on the last known heights and preventing it from being set below a minimum threshold of 0.5 meters.
 *
 * @param old_height The height recorded before the ultrasonic measurement became valid.
 * @param lastUltrasonicDistanceWasValid Indicates whether the last ultrasonic measurement before the current one was valid.
*/
void handle_validity_of_required_height(float old_height, bool lastUltrasonicDistanceWasValid){
  if (USValidityRate <= 0.4 && !lastUltrasonicDistanceWasValid && ultrasonicDistanceIsValid && !DoNotMove && !LAND) {
    float r;
    if (old_height >= REQ_HEIGHT){
      r = CURRENT_HEIGHT - abs(old_height - REQ_HEIGHT);
    }
    else{
      r = CURRENT_HEIGHT + abs(REQ_HEIGHT - old_height);
    }

    REQ_HEIGHT = (r > 0.5) ? r : 0.5;
  }
  else if(LAND){
    if(ultrasonicDistanceIsValid && CURRENT_HEIGHT > 1 && !(REQ_HEIGHT >= 0.4 && REQ_HEIGHT < 0.6)){
      REQ_HEIGHT = 0.5;
    }else if(ultrasonicDistanceIsValid && CURRENT_HEIGHT < 1 && !(REQ_HEIGHT >= 0.04 && REQ_HEIGHT < 0.06)){
      REQ_HEIGHT = 0.05;
    }else if(USValidityRate == 0 && (CURRENT_HEIGHT - REQ_HEIGHT) < 1){
      REQ_HEIGHT -= 1;
    }else if(USValidityRate >= 0.85 && ultrasonicDistanceIsValid && CURRENT_HEIGHT < 0.1){
      DoNotMove = true;
    }
  }
}

/**
 * Display measured values.
*/
void display_values(void) {
  Serial.println();
  Serial.print("CurrentHeight : "); Serial.println(CURRENT_HEIGHT);
  Serial.print("RequiredHeight: "); Serial.println(REQ_HEIGHT);
  Serial.print("Ultrasonic : "); Serial.println(ultrasonic_distance);
  Serial.print("Humidity : "); Serial.println(humidity);
  Serial.print("Temperature : "); Serial.println(temperature_DHT22);
  Serial.print("Pressure : "); Serial.println(pressure);
  Serial.print("Altitude : "); Serial.println(altitude, 2);
  Serial.print("Temperature_MPL3115A2 : "); Serial.println(temperature_MPL3115A2);
  // Display GPS data
  Serial.println("------------- GPS Acquired data");
  Serial.print("Altitude: "), Serial.println(altitudeGPS);
  Serial.print("Speed: "), Serial.println(speed);
  Serial.print("latitude: "), Serial.println(latitude, 6);
  Serial.print("longitude: "), Serial.println(longitude, 6);
  Serial.print("Time: "), Serial.println(tm);
  Serial.println("-------------");
  Serial.println();
}

void ControlSignalingDiode(void){
  if (LED_shine && abs(millis() - lastLedTime) >= 500){
    digitalWrite(LED, LOW);
    LED_shine = false;
  } else if (!LED_shine && abs(millis() - lastLedTime) >= 1500) {
    digitalWrite(LED, HIGH);
    LED_shine = true;
    lastLedTime = millis();
  }
}