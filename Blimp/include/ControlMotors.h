#ifndef ControlMotors_h
#define ControlMotors_h

#include "GeneralLib.h"
#include <Servo.h>
#include "PID.h"

// motors pins
const unsigned int ESC_H_1 = 8; // altitude control engine 1
const unsigned int ESC_H_2 = 20; // altitude control engine 2
const unsigned int FORWARD_ESC = 23; // direction control engine
const unsigned int SERVO_PIN = 22;

void InitializeESCs(void);
void InitializeSERVO(void);
void ControlSteeringMotor(void);
void ControlServo(float angle);
void ControlHeight(void);
void InputShaping(float &CurrentRequiredHeight, float &LastRequiredHeight, float TargetRequiredHeight, float RateOfChange, float SampleTimeInSec, float alpha);
void IntegrateInput(float &CurrentRequiredHeight, float TargetRequiredHeight, float RateOfChange, float dt);
void FirstOrderFilter(float &CurrentRequiredHeight, float LastRequiredHeight, float alpha);
void DeadZoneControll(float RequiredHeight, float& Force, float& Thrust, float& PW, bool& Manual);
void AutomaticControl(float RequiredHeight, float& Force, float& Thrust, float& PW);
float force_to_thrust(float val);
int thrust_to_PWM(float thrust);
void WaitWhileDoNotMove(void);

#endif
