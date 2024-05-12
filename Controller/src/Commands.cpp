#include "Commands.h"

commandID all_ids;
BalloonREPORT report;

/**
 * Reads user input and generates commands for balloon control.
 * Handles button presses and serial input.
 *
 * @param cmd Reference to the command structure where the generated command is stored.
 * @return Returns true if a new command is generated, otherwise false.
 */
bool get_command(command& cmd){

  bool neww = false;

  if (!digitalRead(LandButton) and (millis() - timeOfLastCommand) > buttonInterval){
    process_command(cmd, all_ids.LAND, neww);
  }else if(Serial.available()){
    get_command_from_serial(cmd, neww);
  }else if(!digitalRead(UpButton) and (millis() - timeOfLastCommand) > buttonInterval){
    process_command(cmd, all_ids.UP, neww);
  }else if(!digitalRead(DownButton) and (millis() - timeOfLastCommand) > buttonInterval){
    process_command(cmd, all_ids.DOWN, neww);
  }else if(!digitalRead(FlyForwardButton) and (millis() - timeOfLastCommand) > buttonInterval) {
    process_command(cmd, all_ids.FLY_FORWARD, neww);
  }else if(angle_change()){
    process_command(cmd, all_ids.POTENTIOMETER_ANGLE, neww, last_angle);
  }else if((millis() - lastSendTime) > timeInterval){
    process_command(cmd, all_ids.SAY_HI, neww);
  }

  if(neww && cmd.ID != all_ids.SAY_HI){
    timeOfLastCommand = millis();
  }
  return neww;
}

/**
 * Processes commands received from the serial port.
 * Commands can be either textual or numerical. Numerical command is in format "name | value".
 *
 * @param cmd Reference to the command structure where the generated command is stored.
 * @param neww Reference to a boolean value, set to true if a new command is received.
 */
void get_command_from_serial(command& cmd, bool& neww){
  String input = read_serial_input();
  String lowerInput = input.toLowerCase();

  int index = lowerInput.indexOf("|"); // Pokud je zadána hodnota je v první části její název a až ve druhé samostatná hodnota. Tyto části jsou odděleny svislou čárou.

  if (index != -1){
    process_value_input(lowerInput, cmd, neww, index);
  }else{
    // Process text commands
    process_text_input(lowerInput, cmd, neww);
  }
}

/**
 * Reads the incoming serial data
 *
 * @return A String containing the concatenated serial data.
 */
String read_serial_input() {
    String input = "";
    while (Serial.available()) {
        input += (char)Serial.read();
    }
    return input;
}

/**
 * Processes value commands received from the serial input.
 * Updates the command structure and variables based on the received command.
 *
 * @param lowerInput A lowered input string received from the serial.
 * @param cmd A reference to the command structure where the command details are stored.
 * @param neww A reference to a boolean flag indicating if a new command is received.
 * @param index An index of "|" in received string.
 */
void process_value_input(const String& lowerInput, command& cmd, bool& neww, int index) {
  String text = lowerInput.substring(0, index);
  text.replace(" ", ""); // Replace spaces
  float value = lowerInput.substring(index + 1).toFloat(); // +1 for skipping "|"

  if (text == "height" && value != 0) {
    process_command(cmd, all_ids.SET_EXACT_HEIGHT, neww, value);
    Serial.print("NEW HEIGHT: "), Serial.print(value), Serial.println(" [m]");
  } else if (text == "power" && value != 0) {
    process_command(cmd, all_ids.SET_MOTOR_POWER, neww, value);
    Serial.print("NEW POWER OF STEERING MOTOR: "), Serial.print(value), Serial.println(" [-]");
  } else {
    Serial.println("INVALID COMMAND!");
  }
}

/**
 * Processes text commands received from the serial input.
 * Converts the input to lowercase and checks for specific commands like 'land', 'up', 'down', and 'help'.
 * Updates the command structure and variables based on the received command.
 *
 * @param lowerInput A lowered input string received from the serial.
 * @param cmd A reference to the command structure where the command details are stored.
 * @param neww A reference to a boolean flag indicating if a new command is received.
 */
void process_text_input(const String& lowerInput, command& cmd, bool& neww) {
    if (lowerInput == "land" || lowerInput == "land\n") {
      process_command(cmd, all_ids.LAND, neww);
    } else if (lowerInput == "up" || lowerInput == "up\n") {
      process_command(cmd, all_ids.UP, neww);
    } else if (lowerInput == "down" || lowerInput == "down\n") {
      process_command(cmd, all_ids.DOWN, neww);
    } else if (lowerInput == "forward" || lowerInput == "forward\n"){
      process_command(cmd, all_ids.FLY_FORWARD, neww);
    } else if (lowerInput == "centre" || lowerInput == "centre\n"){
      center_it(); // center the encoder.
      Serial.println("The rotary potentiometer (steering encoder) has been centered.");
    } else if(lowerInput == "off" || lowerInput == "off\n") {
      process_command(cmd, all_ids.MOTORS_OFF, neww);
    }else if (lowerInput == "help" || lowerInput == "help\n") {
        display_help();
    } else {
        Serial.println("Invalid command was entered. If you want to help, type 'help' in Serial Monitor.");
    }
}

/**
 * Processes a command based on the provided command ID. 
 * It resets the counter if needed, updates the command structure with the new command ID and counter, 
 * and sets the flag indicating that a new command has been processed.
 *
 * @param cmd A reference to the command structure where the command details are stored.
 * @param commandID The ID of the command to be processed.
 * @param neww A reference to a boolean flag indicating if a new command is received.
 */
void process_command(command& cmd, int commandID, bool& neww, float value = -1) {
  reset_counter_if_needed();
  command temporary_cmd(commandID, counter, value);
  cmd = temporary_cmd;
  neww = true;
}

/**
 * Resets the global counter to zero if it reaches its maximum value (127).
 * This ensures that the counter always fits into one byte, useful for packet size constraints.
 */
void reset_counter_if_needed() {
    if (counter++ >= 127) {
        counter = 0;
    }
}

/**
 * Displays help information on the serial monitor.
 * Provides guidance on the commands that can be used to control the balloon.
 */
void display_help() {
  Serial.println();
  Serial.print("If you want the Airship to: "), Serial.print("start/stop landing, type "), Serial.println("'land'.");
  Serial.print("                            "), Serial.print("drop down, type "), Serial.println("'down'.");
  Serial.print("                            "), Serial.print("rise higher, type "), Serial.println("'up'.");
  Serial.print("                            "), Serial.print("start/stop flying forward, type "), Serial.println("'forward'.");
  Serial.println();
  Serial.println("If you want to set height manually, enter 'height' followed by the desired height in metres, separated by '|'. E.g. 'height | 6.2' to set 6 metres and 20 centimetres.");
  Serial.println();
  Serial.println("If you want to set power of steering motor manually, enter 'power' followed by the desired power level (between 0.1 and 180), separated by '|'. E.g. 'power | 180' for full power.");
  Serial.println();
  Serial.println("If you want to centre the rotation potentiometer (rotary encoder) type 'centre'.");
  Serial.println();
  Serial.println("Another option is to control the balloon directly from the controller.");
  Serial.println();
}