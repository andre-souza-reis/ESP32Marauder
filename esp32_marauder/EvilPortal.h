#pragma once

#ifndef EvilPortal_h
#define EvilPortal_h

#include "configs.h"
#include "settings.h"

#include "Buffer.h"
#include "lang_var.h"

extern Settings settings_obj;

extern Buffer buffer_obj; 

struct ssid {
  String essid;
  uint8_t channel;
  uint8_t bssid[6];
  bool selected;
};

struct AccessPoint {
  String essid;
  uint8_t channel;
  uint8_t bssid[6];
  bool selected;
  LinkedList<char>* beacon;
  char rssi;
  LinkedList<uint8_t>* stations;
};

#endif