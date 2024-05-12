#ifndef GeneralLib_h
#define GeneralLib_h

#include <Arduino.h>
#include <SPI.h>
#include <TeensyThreads.h>
#include "Potentiometer.h"

extern unsigned char localAddress;         // address of this device
extern unsigned char destinationAddress;   // address of reciver

const int ERR_VALUE = INT16_MIN;

struct commandID {
  static const unsigned char LAND = 0xFF;
  static const unsigned char UP = 0xF0;
  static const unsigned char DOWN = 0x0F;
  static const unsigned char SET_EXACT_HEIGHT = 0xCC; // if user want to set exact height 
  static const unsigned char SAY_HI = 0x33;
  static const unsigned char POTENTIOMETER_ANGLE = 0x66; // Control the rotation of the airship's steering motor
  static const unsigned char FLY_FORWARD = 0x99; // Switch on/off the steering motor
  static const unsigned char SET_MOTOR_POWER = 0x3C; // Adjusts the power of the steering motor
  static const unsigned char MOTORS_OFF = 0x55;
};extern commandID all_ids;

struct BalloonREPORT {
  static const unsigned char CONFIRMATION = 0x00;
  static const unsigned char MEASURED_DATA = 0xFF;
};extern BalloonREPORT report;

class command {
  public:
    unsigned char ID; // Command ID
    int value;
    unsigned int counter; // The command must also have a special counter. This is in case the message gets lost or damaged on the way.
    command(unsigned char type, unsigned int cnt, float val = ERR_VALUE) {
      commandID ids;
      counter = cnt;
      if (type == ids.SET_EXACT_HEIGHT){
        if (val != ERR_VALUE){
          ID = type;
          value = val*100; // Convert input value in m to cm
        } 
        else{
          ID = ids.SAY_HI;
          Serial.println("INVALID COMMAND!");
        }
      }else if(type == ids.POTENTIOMETER_ANGLE){
        if (abs(val) <= 270){ // [degrees]
          ID = type;
          value = val;
        }else{
          ID = ids.SAY_HI;
          Serial.println("INVALID COMMAND!");
        }
      }else if(type == ids.SET_MOTOR_POWER){
        if (val >= 0 && val <= 180){
          ID = type;
          value = val;
        }else{
          ID = ids.SAY_HI;
          Serial.println("INVALID COMMAND!");
        }

      }else {
        ID = type;
        value = 0; // TODO
      }
    }
};

extern unsigned int counter;         // msg counter
extern long lastSendTime;            // last send time
extern long lastReceivedTime;        // last received time
extern long timeOfLastCommand;       // time at witch last command was entered
extern int timeInterval;          // Interval between sending times
extern int buttonInterval;         // Interval between button presses
extern bool lastCmdConfirmed;     // true if last command was confirmed by balloon
extern bool confirmation_arrived; // true if answer(confirmation) from balloon arrived

extern bool LAND;                // Is set by return msg from balloon
extern bool FLY_FORWARD;           // true if steering motor of airshit is on

#endif