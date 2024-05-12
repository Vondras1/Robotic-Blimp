#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "GeneralLib.h"
#include <LoRa.h>

const int csPin = 9;          // LoRa radio chip select //3
const int resetPin = 15;       // LoRa radio reset //1
const int dio0 = 14;         // hardware interrupt pin //0

struct commandIDs {
  static const unsigned char LAND = 0xFF;
  static const unsigned char UP = 0xF0;
  static const unsigned char DOWN = 0x0F;
  static const unsigned char SET_EXACT_HEIGHT = 0xCC; // if user want to set exact height 
  static const unsigned char SAY_HI = 0x33;
  static const unsigned char POTENTIOMETR_ANGLE = 0x66; // Řídí natočení řídícího motoru vzducholodě
  static const unsigned char FLY_FORWARD = 0x99; // Zapni/vypni řídící motor
  static const unsigned char SET_MOTOR_POWER = 0x3C; // Nastavuje vykon zataceciho motoru
  static const unsigned char MOTORS_OFF = 0x55;
};
extern commandIDs all_ids;

struct BalloonREPORT {
  static const unsigned char CONFIRMATION = 0x00;
  static const unsigned char MEASURED_DATA = 0xFF;
}; 
extern BalloonREPORT report;

const unsigned char localAddress = 0x11;     // address of this device
const unsigned char destinationAddress = 0xBB;      // address of reciver

void onReceive(int packetSize);
bool MsgIsForMe(unsigned char recipientAddres);
bool check_parity(unsigned int number, bool originalParity);
bool check_cmdID(unsigned char cmdID);
bool handleDuplicateMessage(bool valid_msg, unsigned char cmdID, unsigned int counter);
void read_specific_value(bool &valid_msg, unsigned char cmdID);
void sendMessage(String msg, unsigned char type_of_msg, bool land, bool fly_forward, unsigned int cnt = 127);
unsigned char encode_to_byte(bool x, bool y, bool z, bool w);
void send_measured_data(void);
char createCounterByte(unsigned int counter);
void ExtractCounterFromByte(unsigned int &counter, bool &counterParity, unsigned char rByte);
bool get_even_parity(unsigned int number);
void process_command(void);
void SetLAND(void);
void set_new_required_height(void);
void decodeValueAndParity(int receivedValue ,int &newValue, bool &originalParity);

#endif // COMMUNICATION_H