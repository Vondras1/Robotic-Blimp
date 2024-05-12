#include "LedDiods.h"

bool LAND_LED_shine = false;      // true if land led shine
bool UP_LED_shine = false;        // true if up led shine
bool DOWN_LED_shine = false;      // true if down led shine
bool FLY_FORWARD_LED_shine = false; // true if fly_forward led shine

/**
 * Controls the LED indicators based on the command and system state.
 * It includes logic for LAND, UP, and DOWN LEDs.
 *
 * @param cmd A reference to the command structure containing the current command.
 * @param neww A boolean indicating if a new command has been received.
 */
void control_diodes(command& cmd, bool neww) {
  update_LAND_LED();
  update_FORWARD_FLY_LED();
  if (lastCmdConfirmed) {
    turn_off_LEDS_after_interval();
    light_up_after_new_cmd(cmd, neww);
  }
}

/**
 * Updates the state of the LAND LED based on the LAND status.
 */
void update_LAND_LED() {
  if (LAND && !LAND_LED_shine) {
    digitalWrite(LAND_LED, HIGH);
    LAND_LED_shine = true;
  }else if (!LAND && LAND_LED_shine) {
    digitalWrite(LAND_LED, LOW);
    LAND_LED_shine = false;
  }
}

/**
 * Updates the state of the FORWARD_FLY LED based on the FORWARD_FLY status.
 */
void update_FORWARD_FLY_LED() {
  if (FLY_FORWARD && !FLY_FORWARD_LED_shine) {
    digitalWrite(FLY_FORWARD_LED, HIGH);
    FLY_FORWARD_LED_shine = true;
  }else if (!FLY_FORWARD && FLY_FORWARD_LED_shine) {
    digitalWrite(FLY_FORWARD_LED, LOW);
    FLY_FORWARD_LED_shine = false;
  }
}

/**
 * Turns off UP and DOWN LEDs after a specified interval.
 */
void turn_off_LEDS_after_interval() {
  if ((UP_LED_shine || DOWN_LED_shine) && (millis() - timeOfLastCommand) > buttonInterval) {
    digitalWrite(UP_LED, LOW);
    UP_LED_shine = false;
    digitalWrite(DOWN_LED, LOW);
    DOWN_LED_shine = false;
  }
}

/**
 * Lights up the corresponding LED based on the current command.
 *
 * @param cmd The current command.
 * @param neww Indicates if a new command has been received.
 */
void light_up_after_new_cmd(const command& cmd, bool neww) {
  if (neww) {
    if (cmd.ID == all_ids.DOWN) {
      digitalWrite(DOWN_LED, HIGH);
      DOWN_LED_shine = true;
    }else if (cmd.ID == all_ids.UP) {
      digitalWrite(UP_LED, HIGH);
      UP_LED_shine = true;
    }
  }
}
