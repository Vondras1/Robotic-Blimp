#ifndef COMMANDS_H
#define COMMANDS_H

#include "GeneralLib.h"

const unsigned int LandButton = 8;
const unsigned int UpButton = 20;
const unsigned int DownButton = 21;
const unsigned int FlyForwardButton = 4;

bool get_command(command& cmd);
void process_command(command& cmd, int commandID, bool& neww, float value = -1);
void get_command_from_serial(command& cmd, bool& neww);
String read_serial_input(void);
void reset_counter_if_needed(void);
void process_value_input(const String& lowerInput, command& cmd, bool& neww, int index);
void process_text_input(const String& lowerInput, command& cmd, bool& neww);
void display_help(void);


#endif // COMMANDS_H

