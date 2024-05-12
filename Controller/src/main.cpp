/**
 * Project: Robotic Blimp
 * Author: Vojtěch Vondráček
 * Date: 10.5.2024
 * Description: Implementation of the controller used for communication with the blimp.
 * Note: Some comments were co-created by generative artificial intelligence.
 */

#include "GeneralLib.h"
#include "Communication.h"
#include "Commands.h"
#include "LedDiods.h"

void setup() {
  Serial.begin(9600); // initialize serial

  // overwrite the default CS, reset, and IRQ pins (defined in Communication.h)
  LoRa.setPins(csPin, resetPin, dio0);

  // initialize ratio at 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa - Controller init failed. Check connections.");
    while (true); // if failed, do nothing
  }

  // Initilize the push button pins as an input
  pinMode(LandButton, INPUT_PULLUP);
  pinMode(UpButton, INPUT_PULLUP);
  pinMode(DownButton, INPUT_PULLUP);
  pinMode(FlyForwardButton, INPUT_PULLUP);

  // Initialize the led pins as outputs
  pinMode(LAND_LED, OUTPUT);
  pinMode(UP_LED, OUTPUT);
  pinMode(DOWN_LED, OUTPUT);
  pinMode(FLY_FORWARD_LED, OUTPUT);

  // Set led pins to LOW
  digitalWrite(LAND_LED, LOW);
  digitalWrite(UP_LED, LOW);
  digitalWrite(DOWN_LED, LOW);
  digitalWrite(FLY_FORWARD_LED, LOW);

  SetCentreValue();

  threads.addThread(thread_upgrade_angle);

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("Controller init succeeded.");  
}

command cmd(0x00, 0); // command
void loop() {
  control_diodes(cmd, false);
  if (lastCmdConfirmed ){ // The previous command has already been processed and a new one has just been entered.
    bool neww = get_command(cmd);
    if(neww){
      control_diodes(cmd, neww);
      if(cmd.ID == all_ids.SAY_HI){ 
        // With no acknowledgment of receipt
        send_command(cmd);
        Serial.println("Sent the informational command 'SAY_HI'.");
      }else{ 
        // With acknowledgment of receipt
        send_command(cmd);
        lastCmdConfirmed = false;
        confirmation_arrived = false;
        Serial.print("First attempt sent. Command counter is - "), Serial.print(cmd.counter), Serial.print(",  command ID - "), Serial.println(cmd.ID);
      }
      lastSendTime = millis();
      LoRa.receive();
    }
  }else{
    if(confirmation_arrived or ((millis() - lastSendTime) > 500)){
      send_command(cmd);
      confirmation_arrived = false;
      lastSendTime = millis();
      Serial.print("Another attempt sent. Command counter - "), Serial.print(cmd.counter), Serial.print(", command ID - "), Serial.println(cmd.ID);
      Serial.println();
      LoRa.receive();
    }
  }
}