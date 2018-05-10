// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran
#ifndef IR_GREE_H_
#define IR_GREE_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"
#include "IRsend.h"

#define GREE_HDR_MARK      9000U
#define GREE_HDR_SPACE     4000U
#define GREE_BIT_MARK       620U
#define GREE_ONE_SPACE     1600U
#define GREE_ZERO_SPACE     540U
#define GREE_MSG_SPACE    19000U

// Power state
#define GREE_POWER_OFF  0x00
#define GREE_POWER_ON   0x08

// Operating modes
// Gree codes
#define GREE_MODE_AUTO  0x00
#define GREE_MODE_HEAT  0x04
#define GREE_MODE_COOL  0x01
#define GREE_MODE_DRY   0x02
#define GREE_MODE_FAN   0x03

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define GREE_FAN_AUTO   0x00 // Fan speed
#define GREE_FAN1       0x10 // * low
#define GREE_FAN2       0x20 // * med
#define GREE_FAN3       0x30 // * high


#define GREE_MIN_TEMP 16U
#define GREE_MAX_TEMP 30U

// Vertical air directions. Note that these cannot be set on all heat pumps
#define GREE_VDIR_AUTO   0
#define GREE_VDIR_MANUAL 0
#define GREE_VDIR_SWING  1
#define GREE_VDIR_UP     2
#define GREE_VDIR_MUP    3
#define GREE_VDIR_MIDDLE 4
#define GREE_VDIR_MDOWN  5
#define GREE_VDIR_DOWN   6

#if SEND_GREE
class IRGree {
 public:
  explicit IRGree(uint16_t pin);
  void checksum()
  void stateReset();
  void send();
  void begin();
  void on();
  void off();
  void setPower(bool state);
  bool getPower();
  void setTemp(uint8_t temp);
  uint8_t getTemp();
  void setFan(uint8_t fan);
  uint8_t getFan();
  void setMode(uint8_t mode);
  uint8_t getMode();
  void setSwingV(uint8_t mode)
  uint8_t SwingV();
  uint8_t* getRaw();

 private:
  uint8_t remote_state[GREE_STATE_LENGTH];
  IRsend _irsend;
};

#endif








