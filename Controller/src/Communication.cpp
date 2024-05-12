#include "Communication.h"

bool LAND = false;
bool FLY_FORWARD = false;

unsigned char localAddress = 0xBB;         // address of this device
unsigned char destinationAddress = 0x11;   // address of reciver

unsigned int counter = 0;         // msg counter
long lastSendTime = 0;            // last send time
long lastReceivedTime = 0;        // last received time
long timeOfLastCommand = 0;       // time at witch last command was entered
int timeInterval = 5400;          // Interval between sending times
int buttonInterval = 800;         // Interval between button presses
bool lastCmdConfirmed = true;     // true if last command was confirmed by balloon
bool confirmation_arrived = true; // true if answer(confirmation) from balloon arrived

/**
 * Handles the incoming LoRa messages. It processes different types of messages based on their format and content.
 * If the message is not for this device or if the packet size is zero, the function returns without processing.
 * It also checks for parity to ensure data integrity.
 * 
 * @param packetSize The size of the incoming packet.
 */
void onReceive(int packetSize) {
  if (packetSize == 0) return;  // exit the function if no packet received

  unsigned char recipientAddres = LoRa.read(); // recipient address
  if (!MsgIsForMe(recipientAddres)) return;  // if not for this device, exit
  lastReceivedTime = millis(); // update time of the last received message

  unsigned char sender = LoRa.read();  // sender address

  // Process airship status message
  process_airship_status();

  // Determine the type of the message and handle accordingly
  unsigned char type_of_msg = LoRa.read();
  if (type_of_msg == report.CONFIRMATION){
    handle_confirmation_message();
  }else if (type_of_msg == report.MEASURED_DATA){
    handle_measured_data_message();
  }else{
    Serial.println("Corrupted or unknown message type.");
  }
}

/**
 * Determines if a received message is intended for this device.
 * 
 * @param recipientAddres The address of the message's recipient.
 * @return True if the message is for this device, False otherwise.
 */
bool MsgIsForMe(byte recipientAddres) {
  if (recipientAddres != localAddress) {
    Serial.println("This message is not for me.");
    return false;                             
  }
  return true; // The message is for this device
}

/**
 * Processes actual airship status message received from the LoRa module. 
 * Reads actual states and its parity bit, checks for parity correctness
 * and updates the global variable accordingly.
 */
void process_airship_status(void) {
  bool landingStatus = 0;
  bool landingParity = 0;
  bool flyingStatus = 0;
  bool flyingParity = 0;

  unsigned char one_byte = LoRa.read();
  decode_from_byte(landingStatus, landingParity, flyingStatus, flyingParity, one_byte);
  updates_landing_status(landingStatus, landingParity);
  updates_flying_status(flyingStatus, flyingParity);
}

/**
 * Function to read the values of boolean variables x, y, z, w from a single unsigned char variable one_byte.
*/
void decode_from_byte(bool &x, bool &y, bool &z, bool &w, unsigned char one_byte){
  x = (one_byte >> 6) & 1;
  y = (one_byte >> 4) & 1;
  z = (one_byte >> 2) & 1;
  w = one_byte & 1;
}

/**
 * Checks for parity correctness and updates the global landing status accordingly.
 */
void updates_landing_status(bool landingStatus, bool landingParity) {
  if (check_parity(landingStatus, landingParity)) {
      LAND = (landingStatus == 1); // update landing status
  }
}

/**
 * Checks for parity correctness and updates the global flying forward status accordingly.
 */
void updates_flying_status(bool flyingStatus, bool flyingParity) {
  if (check_parity(flyingStatus, flyingParity)) {
      FLY_FORWARD = (flyingStatus == 1); // update flying status
  }
}

/**
 * Handles confirmation messages received from the LoRa module.
 * Reads the confirmation counter and its parity bit, checks for parity correctness,
 * and validates the confirmation. If valid, updates global confirmation variable.
 */
void handle_confirmation_message() {
  unsigned int confirmedCounter; 
  bool counterParity;
  ExtractCounterFromByte(confirmedCounter, counterParity, LoRa.read());
  if (check_parity(confirmedCounter, counterParity)) {
    String confirmationResponse = read_LoRa_string(); // Read the rest of the message

    if (confirmationResponse == "VALID" && confirmedCounter == counter) {
        lastCmdConfirmed = true;
        confirmation_arrived = true;
        Serial.println("Confirmed");
    } else {
        Serial.print("--- ERROR --- Counter: "), Serial.print(confirmedCounter);
        Serial.print(", Response: "), Serial.println(confirmationResponse);
    }
  }
}

/**
 * Handles measured data messages received from the LoRa module.
 * Reads the entire data message as a string and prints it to the serial monitor.
 */
void handle_measured_data_message() {
    String dataReport = read_LoRa_string(); // Read the data report
    Serial.println("---"); // start new data
    Serial.print(dataReport);
    Serial.println("+++"); // end new data
}

/**
 * Reads a string from the LoRa module.
 * 
 * @return The concatenated string from the LoRa module.
 */
String read_LoRa_string() {
    String data = "";
    while (LoRa.available()) {
        data += (char)LoRa.read(); // Concatenate bytes into a string
    }
    return data;
}

/**
 * Sends a command using LoRa communication in a specific format.
 * Format: [recipient address, local address, counter, parity for counter, command ID, optionally (parity bit, required height)]
 * Note: When using "LoRa.write()", the argument must be less than or equal to one byte.
 * 
 * @param cmd The command structure containing the details of the command to be sent.
 */
void send_command(const command& cmd) {
  LoRa.beginPacket();                        // start packet
  LoRa.write(destinationAddress);            // add destination address
  LoRa.write(localAddress);                  // add sender's (local) address
  LoRa.write(createCounterByte(cmd.counter));           // add counter and its parity
  LoRa.write(cmd.ID);                        // add command ID to identify the command type

  // For specific commands, add additional data and parity bit
  if (cmd.ID == all_ids.SET_EXACT_HEIGHT || cmd.ID == all_ids.POTENTIOMETER_ANGLE || cmd.ID == all_ids.SET_MOTOR_POWER) {
    LoRa.println(GetValueWithParity(cmd.value));
  }
  LoRa.endPacket();                          // finish and send the packet
}

/**
 * Combines a counter value with a parity bit.
 * 
 * @param counter The counter value to be encoded.
 * @return Returns the encoded counter as an unsigned char, with parity information in the MSB.
 */
unsigned char createCounterByte(unsigned int counter){
  unsigned char inp = (unsigned char)(counter & 0x7F);
  bool parityBit = get_even_parity(counter);
  if (parityBit)
    inp |= 0x80; // set MSB to 1
  return inp;
}

/**
 * Extracts counter value and its parity from a received byte.
 * 
 * @param counter Reference to store the extracted counter.
 * @param counterParity Reference to store the extracted parity.
 * @param rByte The byte from which the counter and parity are extracted.
 */
void ExtractCounterFromByte(unsigned int &counter, bool &counterParity, unsigned char rByte){
  counter = rByte & 0x7F;
  counterParity = (rByte >> 7) & 1;
}

/**
 * Encodes an integer value by appending a parity bit.
 * This function calculates the even parity for the input value, then shifts the value left by 1 bit and appends the parity bit at the least significant bit.
 * 
 * @param value The integer value to encode with a parity bit.
 * @return Returns the value encoded with a parity bit.
 */
int GetValueWithParity(int value){
  bool parity = get_even_parity((unsigned int)value);
  int modifiedNumber = (value << 1) | parity;
  return modifiedNumber;
}

/**
 * Calculates the even parity bit for a given number.
 * 
 * @param number The number for which to calculate the parity bit.
 * @return The calculated parity bit (0 or 1).
 */
bool get_even_parity(unsigned int number) {
  bool parityBit = 0;
  while (number) {
    parityBit ^= number & 1; // XOR operation between the least significant bit and the current parity bit
    number >>= 1;  /// Shift the number right by one bit to process the next bit
  }

  return parityBit;
}

/**
 * Checks if the parity of a given number matches the provided original parity.
 * 
 * @param number The number whose parity is to be checked.
 * @param originalParity The original parity bit received with the number.
 * @return True if the calculated parity matches the original parity, False otherwise.
 */
bool check_parity(unsigned int number, bool originalParity) {
  bool currentParity = get_even_parity(number);
  if (currentParity == originalParity){return true;}
  else {return false;}
}

/*
NOTES:

reading
  LoRa.read() - reads one byte of the message
  available() - returns the number of bytes available to read

writing
  LoRa.beginPacket()
  LoRa.write() - writes one byte of the message
  LoRa.print() - splits an arbitrarily long message into individual bytes and then sends them
  LoRa.endPacket()

In the onReceive(int packetSize) function, the "packetSize" variable indicates the number of bytes in the received message.

setFrequency() - sets the frequency of the LoRa module

If I want to increase the range, it might be a good idea to expand the bandwidth, but this also worsens the noise.
  setSignalBandwidth - sets the bandwidth (1-9) = (7.8, 10.4, 15.6, 20.8, ..., 62.5, 125, 250, 500) [kHz]
  getSignalBandwidth - returns the current bandwidth

LoRa.setPayloadLength(10); // Sets a fixed packet length of 10 bytes

setGain() - antenna gain setting
*/