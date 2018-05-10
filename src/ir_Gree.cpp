// Copyright 2017 Ville Skyttä (scop)
// Copyright 2017 David Conran
//
//  Gree protocol compatible heat pump carrying the "Ultimate" brand name.
//

#include "ir_Gree.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"

//                      GGGG  RRRRRR  EEEEEEE EEEEEEE
//                     GG  GG RR   RR EE      EE
//                    GG      RRRRRR  EEEEE   EEEEE
//                    GG   GG RR  RR  EE      EE
//                     GGGGGG RR   RR EEEEEEE EEEEEEE

// Constants
// Ref: https://github.com/ToniA/arduino-heatpumpir/blob/master/GreeHeatpumpIR.h
#define GREE_HDR_MARK      9000U
#define GREE_HDR_SPACE     4000U
#define GREE_BIT_MARK       620U
#define GREE_ONE_SPACE     1600U
#define GREE_ZERO_SPACE     540U
#define GREE_MSG_SPACE    19000U

#if SEND_GREE
// Send a Gree Heat Pump message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=GREE_STATE_LENGTH)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/ToniA/arduino-heatpumpir/blob/master/GreeHeatpumpIR.cpp
void IRsend::sendGree(unsigned char data[], uint16_t nbytes, uint16_t repeat) {
  if (nbytes < GREE_STATE_LENGTH)
    return;  // Not enough bytes to send a proper message.

  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header #1
    mark(GREE_HDR_MARK);
    space(GREE_HDR_SPACE);

    // Data #1
    uint16_t i;
    for (i = 0; i < 4 && i < nbytes; i++)
      sendData(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_BIT_MARK, GREE_ZERO_SPACE,
               data[i], 8, false);

    // Footer #1 (010)
    sendData(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_BIT_MARK, GREE_ZERO_SPACE,
             0b010, 3);

    // Header #2
    mark(GREE_BIT_MARK);
    space(GREE_MSG_SPACE);

    // Data #2
    for (; i < nbytes; i++)
      sendData(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_BIT_MARK, GREE_ZERO_SPACE,
               data[i], 8, false);

    // Footer #2
    mark(GREE_BIT_MARK);
    space(GREE_MSG_SPACE);
  }
}

// Send a Gree Heat Pump message.
//
// Args:
//   data: The raw message to be sent.
//   nbits: Nr. of bits of data in the message. (Default is GREE_BITS)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/ToniA/arduino-heatpumpir/blob/master/GreeHeatpumpIR.cpp
void IRsend::sendGree(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits != GREE_BITS)
    return;  // Wrong nr. of bits to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(GREE_HDR_MARK);
    space(GREE_HDR_SPACE);

    // Data
    for (int16_t i = 8; i <= nbits; i += 8) {
      sendData(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_BIT_MARK, GREE_ZERO_SPACE,
               (data >> (nbits - i)) & 0xFF, 8, false);
      if (i == nbits / 2) {
        // Send the mid-message Footer.
        sendData(GREE_BIT_MARK, GREE_ONE_SPACE, GREE_BIT_MARK, GREE_ZERO_SPACE,
                 0b010, 3);
        mark(GREE_BIT_MARK);
        space(GREE_MSG_SPACE);
      }
    }
    // Footer
    mark(GREE_BIT_MARK);
    space(GREE_MSG_SPACE);
  }
}


IRGree::IRGree(uint16_t pin) : _irsend(pin) {
  stateReset();
}

// Reset the state of the remote to a known good state/sequence.
void IRGree::stateReset() {
  // The state of the IR remote in IR code form.
  // Known good state obtained from:
  //   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L108
  // Note: Can't use the following because it requires -std=c++11
  // uint8_t known_good_state[GREE_STATE_LENGTH] = {
  //    0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00,
  //    0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
  remote_state[0] = 0x00;
  remote_state[1] = 0x00;
  remote_state[2] = 0x00;
  remote_state[3] = 0x00;
  remote_state[4] = 0x00;
  remote_state[5] = 0x20;
  remote_state[6] = 0x00;
  remote_state[7] = 0x00;
}

// Configure the pin for output.
void IRGree::begin() {
    _irsend.begin();
}

// Send the current desired state to the IR LED.
void IRGree::send() {
  checksum();   // Ensure correct checksum before sending.
  _irsend.sendGree(remote_state);
}

// Return a pointer to the internal state date of the remote.
uint8_t* IRGree::getRaw() {
  checksum();
  return remote_state;
}

// Calculate the checksum for the current internal state of the remote.
void IRGree::checksum() {
  uint8_t sum = 0;
  // Checksum is simple addition of all previous bytes stored
  // as a 8 bit value.
  remote_state[7] = (
     (remote_state[0] & 0x0F) +
     (remote_state[1] & 0x0F) +
		 (remote_state[2] & 0x0F) +
     (remote_state[3] & 0x0F) +
		 (remote_state[5] & 0xF0) >> 4 +
     (remote_state[6] & 0xF0) >> 4 +
		 (remote_state[7] & 0xF0) >> 4 +
      0x0A) & 0xF0;
}

// Set the requested power state of the A/C to off.
void IRGree::on() {
  // state = ON;
  remote_state[0] = (remote_state[0] & 0xF7) | GREE_POWER_ON;
}

// Set the requested power state of the A/C to off.
void IRGree::off() {
  // state = OFF;
    remote_state[0] = (remote_state[0] & 0xF7) | GREE_POWER_OFF;
}

// Set the requested power state of the A/C.
void IRGree::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

// Return the requested power state of the A/C.
bool IRGree::getPower() {
  return((remote_state[0] & 0x08) != 0);
}


// Set the temp. in deg C
void IRGree::setTemp(uint8_t temp) {
  temp = std::max((uint8_t) GREE_MIN_TEMP, temp);
  temp = std::min((uint8_t) GREE_MAX_TEMP, temp);
  remote_state[1] = temp - GREE_MIN_TEMP;
}

// Return the set temp. in deg C
uint8_t IRGree::getTemp() {
  return(remote_state[1] + GREE_MIN_TEMP);
}


// Set the speed of the fan
void IRGree::setFan(uint8_t fan) {
  // Bounds check
  if (fan > GREE_FAN3)
    fan = GREE_FAN3;  // Set the fan to maximum if out of range.
  remote_state[0] &= 0x0F;  // Clear the previous state
  remote_state[0] |= fan;
}

// Return the requested state of the unit's fan.
uint8_t IRGree::getFan() {
  uint8_t fan = remote_state[0] & 0xF0;
  return fan;
}


// Return the requested climate operation mode of the a/c unit.
uint8_t IRGree::getMode() {
  return((remote_state[0] & 0x7));
}

// Set the requested climate operation mode of the a/c unit.
void IRGree::setMode(uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case GREE_AUTO: break;
    case GREE_COOL: break;
    case GREE_DRY: break;
    case GREE_HEAT: break;
    default: mode = GREE_AUTO;
  }
  remote_state[0] = (remote_state[0] & 0xF8) | mode;
}


//FABYS SONO ARRIVATO QUI


// Set the requested vane operation mode of the a/c unit.
void IRGree::setSwingV(uint8_t mode) {
  switch (mode) {
	case GREE_VDIR_AUTO: break;
	case GREE_VDIR_SWING: break;
	case GREE_VDIR_UP: break;
	case GREE_VDIR_MUP: break;
	case GREE_VDIR_MIDDLE: break;
	case GREE_VDIR_MDOWN: break;
	case GREE_VDIR_DOWN: break;
        default: mode = GREE_VDIR_AUTO;
  }
  remote_state[4] =  mode;

}

// Return the requested vane operation mode of the a/c unit.
uint8_t IRGree::getSwingV() {
  return (remote_state[4]);
}

#endif  // SEND_GREE
