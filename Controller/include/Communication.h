#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "GeneralLib.h"
#include <LoRa.h>

const unsigned int csPin = 10;          // LoRa radio chip select
const unsigned int resetPin = 14;       // LoRa radio reset
const unsigned int dio0 = 15;         // hardware interrupt pin

void onReceive(int packetSize);
bool MsgIsForMe(unsigned char recipientAddres);
void process_airship_status(void);
void decode_from_byte(bool &x, bool &y, bool &z, bool &w, unsigned char one_byte);
void updates_landing_status(bool landingStatus, bool landingParity);
void updates_flying_status(bool flyingStatus, bool flyingParity);
void handle_confirmation_message(void);
void handle_measured_data_message(void);
String read_LoRa_string(void);
void send_command(const command& cmd);
bool get_even_parity(unsigned int number);
bool check_parity(unsigned int number, bool originalParity);
void ExtractCounterFromByte(unsigned int &counter, bool &counterParity, unsigned char inp);
unsigned char createCounterByte(unsigned int counter);
int GetValueWithParity(int value);

#endif // COMMUNICATION_H