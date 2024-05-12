#include "Communication.h"

bool LAND = false;
bool FLY_FORWARD = false;
float REQ_HEIGHT = 0; // Target height in m
float CURRENT_HEIGHT = 0;
float C_R_H = 0;
int ANGLE = 0;
unsigned int LAST_COUNTER = 127;
unsigned char RECEIVED_ID = 0x11;
int RECEIVED_REQ_HEIGHT = 0; // Required height sent from controller
int POWER_OF_STEERING_MOTOR = 110;
bool VALID_MSG = false;
bool NEW_MSG = false;
long lastSendTime = 0;        // last send time
long lastReceivedTime = 0;        // last received time

/**
 * Handles incoming LoRa packets.
 *
 * @param packetSize The size of the received packet.
 */
void onReceive(int packetSize) {
  if (packetSize == 0) return;  // if there's no packet, return
  Serial.print("Message received");

  unsigned char recipientAddres = LoRa.read();  // recipient address
  if (!MsgIsForMe(recipientAddres)){return;}  // if msg is not for me, skip rest of function
  
  lastReceivedTime = millis();  // Record the time when the message was received
  
  unsigned char senderAddres = LoRa.read(); // sender address
  
  unsigned int counter; // counter
  bool counterParity;  // parity for counter
  ExtractCounterFromByte(counter, counterParity, LoRa.read());  // read counter and his parity
  bool valid_msg = check_parity(counter, counterParity);  // If the parities differ, then "valid_msg" will be set to "false"

  unsigned char cmdID = LoRa.read();  // command ID
  valid_msg = valid_msg && check_cmdID(cmdID);  // If cmdID is invalid, then valid_msg will be set to "false"

  if (handleDuplicateMessage(valid_msg, cmdID, counter)) return;  // Exit if a duplicate message was handled

  if (cmdID == all_ids.SET_EXACT_HEIGHT || cmdID == all_ids.POTENTIOMETR_ANGLE || cmdID == all_ids.SET_MOTOR_POWER){
    read_specific_value(valid_msg, cmdID);
  }

  if (cmdID != all_ids.SAY_HI){  // Handle non-"SAY_HI" commands
    RECEIVED_ID = cmdID;
    VALID_MSG = valid_msg;
    NEW_MSG = true;
  }

  if (valid_msg) {
    LAST_COUNTER = counter;
  }
}

/**
 * Checks if the received LoRa message is intended for this device.
 *
 * @param recipientAddress The address of the recipient from the received message.
 * @return true if the message is for this device, false otherwise.
 */
bool MsgIsForMe(unsigned char recipientAddres){
  if (recipientAddres != localAddress) {
    Serial.println("This message is not for me.");
    return false;                             
  }
  else {return true;}
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
 * Checks if the parity of a given number matches the provided original parity.
 * 
 * @param number The number whose parity is to be checked.
 * @param originalParity The original parity bit received with the number.
 * @return True if the calculated parity matches the original parity, False otherwise.
 */
bool check_parity(unsigned int number, bool originalParity){
  bool currentParity = get_even_parity(number);
  if (currentParity == originalParity){return true;}
  else {return false;}
}

/**
 * Checks if the received command ID is a valid command.
 *
 * @param cmdID The command ID to be checked.
 * @return true if the command ID is valid, false otherwise.
 */
bool check_cmdID(unsigned char cmdID){
  if (cmdID == all_ids.DOWN || cmdID == all_ids.LAND || cmdID == all_ids.SAY_HI || cmdID == all_ids.SET_EXACT_HEIGHT || cmdID == all_ids.UP 
  || cmdID == all_ids.POTENTIOMETR_ANGLE || cmdID == all_ids.FLY_FORWARD || cmdID == all_ids.SET_MOTOR_POWER || cmdID == all_ids.MOTORS_OFF){
    return true;
  }
  else {
    return false;
  }
}

/**
 * Handles potential duplicate messages based on command ID and counter.
 *
 * @param valid_msg Indicates whether the message is still valid.
 * @param cmdID The command ID of the received message.
 * @param counter The counter value of the received message.
 * @return true if a duplicate message was detected and handled, false otherwise.
 */
bool handleDuplicateMessage(bool valid_msg, unsigned char cmdID, unsigned int counter) {
    if (valid_msg && LAST_COUNTER == counter && cmdID != all_ids.SAY_HI) {
        Serial.println("Duplicate counter detected");
        sendMessage("VALID", report.CONFIRMATION, LAND, FLY_FORWARD, LAST_COUNTER);
        LoRa.receive();
        return true;  // Indicates a duplicate message was handled
    }
    return false;  // Indicates no duplicate message was found
}

/**
 * Reads and processes specific values from the received LoRa message.
 *
 * This function is called when the command ID indicates that a specific value needs to be read (e.g., SET_EXACT_HEIGHT or POTENTIOMETR_ANGLE).
 * It reads the additional data from the LoRa module, checks its parity, and if valid, processes the data based on the command ID.
 *
 * @param valid_msg Indicates whether the message is valid. It will be updated based on the parity check.
 * @param cmdID The command ID of the received message.
 */
void read_specific_value(bool &valid_msg, byte cmdID) {
  String value = "";
  while (LoRa.available()) {
    value += (char)LoRa.read();      // add bytes one by one
  }
  int receivedValue = value.toInt();
  
  int newValue;
  bool originalParity;
  decodeValueAndParity(receivedValue, newValue, originalParity);

  valid_msg = check_parity(newValue, originalParity) && valid_msg;

  if (valid_msg){
    if (cmdID == all_ids.SET_EXACT_HEIGHT){
      RECEIVED_REQ_HEIGHT = newValue;
    } else if (cmdID == all_ids.POTENTIOMETR_ANGLE){
      ANGLE = newValue;
    } else if (cmdID == all_ids.SET_MOTOR_POWER){
      POWER_OF_STEERING_MOTOR = newValue;
    }
  }
}

/**
 * Decodes a value received that includes a value and a parity bit.
 * This function extracts the parity bit and the actual value from a received integer where
 * the least significant bit is the parity and the other bits represent the value.
 * 
 * @param receivedValue The received integer containing both the value and the parity.
 * @param newValue Reference to store the extracted value.
 * @param originalParity Reference to store the extracted parity.
 */
void decodeValueAndParity(int receivedValue ,int &newValue, bool &originalParity) {
  originalParity = receivedValue & 1;
  newValue = receivedValue >> 1;
}

/**
 * Sends a message using LoRa communication in a specific format.
 * Format: [recipient address, local address, land, parity for land, fly_forward, parity for fly_forward, command ID, 
 * optionally(counter, parity for counter), message]
 *
 * @param msg The message string to be sent.
 * @param type_of_msg The type of the message being sent.
 * @param land A boolean variable indicating whether to land.
 * @param fly_forward A boolean variable indicating whether to fly forward.
 * @param cnt An optional counter value representing the last correctly received message (default is 127).
 */
void sendMessage(String msg, unsigned char type_of_msg, bool land, bool fly_forward, unsigned int cnt = 127) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destinationAddress);       // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(encode_to_byte(land, get_even_parity(land), fly_forward, get_even_parity(fly_forward))); // add land, parity for land, fly_forward and parity for fly_forward
  LoRa.write(type_of_msg);             // add type of msg
  if(type_of_msg == report.CONFIRMATION){
    LoRa.write(createCounterByte(cnt));
  }
  LoRa.print(msg);                      // add payload
  LoRa.endPacket();                     // finish packet and send it
  lastSendTime = millis();
}

/**
 * Function to write the values of boolean variables x, y, z, w into a single unsigned char variable one_byte.
 * format is '0x0y 0z0w'
*/
unsigned char encode_to_byte(bool x, bool y, bool z, bool w){
  unsigned char one_byte = 0; // Initialize to zero

  // Bitwise operations to store the values of variables x, y, z, w into one_byte
  one_byte |= (x << 6);
  one_byte |= (y << 4);
  one_byte |= (z << 2);
  one_byte |= w;

  return one_byte;
}

/**
 * Creates a byte containing a counter and its parity.
 * This function packs a 7-bit counter and a 1-bit parity into an 8
 * 
 * @param counter The counter value to be packed into the byte.
 * @return A byte with the 7 least significant bits as the counter and the most significant bit as the parity.
*/
char createCounterByte(unsigned int counter){
  unsigned char inp = (unsigned char)(counter & 0x7F);
  bool parityBit = get_even_parity(counter);
  if (parityBit)
    inp |= 0x80; // set MSB to 1
  return inp;
}

/**
 * Calculates the even parity bit for a given number.
 * 
 * @param number The number for which to calculate the parity bit.
 * @return The calculated parity bit (0 or 1).
 */
bool get_even_parity(unsigned int number){
  bool parityBit = 0;
  while (number) {
    parityBit ^= number & 1; // XOR operation between the LSB of the current number and the current parity bit
    number >>= 1;  // Shift one bit to the right
  }
  return parityBit;
}

/**
 * Formats and subsequently sends selected measured data.
*/
void send_measured_data(void) {
  String msg = "CurrentHeight : " + String(CURRENT_HEIGHT) + "\n" + 
  "RequiredHeight : " + String(REQ_HEIGHT) + "\n" +
  "CurrentRequiredHeight : " + String(C_R_H) + "\n" +
  "Temperature : " + String(temperature_DHT22) + "\n" +
  "Humidity : " + String(humidity) + "\n" +
  "Power : " + String(POWER) + "\n" + 
  "Altitude : " + String(altitude) + "\n" +
  "Pressure : " + String(pressure) + "\n" +
  "SpeedGPS : " + String(speed) + "\n" + 
  "LatitudeGPS : " + String(latitude) + "\n" + 
  "LongitudeGPS : " + String(longitude) + "\n";

  sendMessage(msg, report.MEASURED_DATA, LAND, FLY_FORWARD, LAST_COUNTER);
}

/**
 * Process incoming commands by adjusting balloon operations and communication states.
 * 
 * Program should enter this function only if the message has not yet been processed. I.e. NEW_MSG == TRUE.
*/
void process_command(void){
  if (VALID_MSG){
    if(LAND == true){
      if(RECEIVED_ID == all_ids.LAND){
        LAND = false;
        //REQ_HEIGHT = (CURRENT_HEIGHT > 0.5) ? CURRENT_HEIGHT : 0.5;
        REQ_HEIGHT = CURRENT_HEIGHT;
        Serial.print("LANDING STOPPED, new height: "), Serial.println(CURRENT_HEIGHT);
      }else if(RECEIVED_ID == all_ids.POTENTIOMETR_ANGLE){
        Serial.print("Rotation of the steering motor: "), Serial.println(ANGLE);
      }else if(RECEIVED_ID == all_ids.FLY_FORWARD){
        FLY_FORWARD = !FLY_FORWARD;
        Serial.print("Flying forward: "), Serial.println(FLY_FORWARD);
      }else if (RECEIVED_ID == all_ids.SET_MOTOR_POWER){
        Serial.print("Power of the steering motor: "), Serial.println(POWER_OF_STEERING_MOTOR);
      }else if(RECEIVED_ID == all_ids.MOTORS_OFF){
        DoNotMove = true;
        Serial.println("Motors are shutting down.");
      }
    }
    else{
      if(RECEIVED_ID == all_ids.LAND){ //Balloon is LANDING
        SetLAND();
        Serial.println("Balloon is LANDING");
      }else if(RECEIVED_ID == all_ids.POTENTIOMETR_ANGLE){
        Serial.print("Rotation of the steering motor: "), Serial.println(ANGLE);
      }else if(RECEIVED_ID == all_ids.FLY_FORWARD){
        FLY_FORWARD = !FLY_FORWARD;
        Serial.print("Flying forward: "), Serial.println(FLY_FORWARD);
      }else if (RECEIVED_ID == all_ids.UP || RECEIVED_ID == all_ids.DOWN || RECEIVED_ID == all_ids.SET_EXACT_HEIGHT){
        set_new_required_height();
      }else if (RECEIVED_ID == all_ids.SET_MOTOR_POWER){
        Serial.print("Power of the steering motor: "), Serial.println(POWER_OF_STEERING_MOTOR);
      }else if(RECEIVED_ID == all_ids.MOTORS_OFF){
        DoNotMove = true;
        Serial.println("Motors are shutting down.");
      }
    }
    // Send a message to the controller to inform him that his message has arrived successfully.
    sendMessage("VALID", report.CONFIRMATION, LAND, FLY_FORWARD, LAST_COUNTER);
  } else{
    sendMessage("ERROR", report.CONFIRMATION, LAND, FLY_FORWARD, LAST_COUNTER);
  }
}

void SetLAND(void){
  LAND = true;
  FLY_FORWARD = false;
  REQ_HEIGHT = 0.5;
}

/**
 * This function sets a new required height based on the received command.
 * The required height can also go to minus (=> from the very principle of the pressure sensor).
*/
void set_new_required_height(void) {
  float current = CURRENT_HEIGHT;
  switch (RECEIVED_ID) {
    // If the UP command is received:
    case all_ids.UP:
      REQ_HEIGHT = (REQ_HEIGHT < current) ? (current + 1) : (REQ_HEIGHT + 1);
      DoNotMove = false;
      Serial.print("New required height has been set: "), Serial.println(REQ_HEIGHT);
      break;
    // If the DOWN command is received:
    case all_ids.DOWN:
      if(DoNotMove && current < 0.5) {
        REQ_HEIGHT = 0;
      } else {
        DoNotMove = false;
        if(ultrasonicDistanceIsValid) {
          float r = current - 1;
          REQ_HEIGHT = (r > 0.5) ? r : 0.5;
        }else {
          REQ_HEIGHT = (REQ_HEIGHT > current) ? (current - 1) : (REQ_HEIGHT - 1);
        }
        Serial.print("New required height has been set: "), Serial.println(REQ_HEIGHT);
        break;
      }
      
    // If a command to set an exact height is received:
    case all_ids.SET_EXACT_HEIGHT:
      if(ultrasonicDistanceIsValid && RECEIVED_REQ_HEIGHT < 0) REQ_HEIGHT = 0;
      else{
        REQ_HEIGHT = float(RECEIVED_REQ_HEIGHT)/100; // conversion to m
        DoNotMove = false;
        Serial.print("New required height has been set: "), Serial.println(REQ_HEIGHT);
      }
      break;
    // If an other command is received, do nothing.
    default:
      break;
    }
}
