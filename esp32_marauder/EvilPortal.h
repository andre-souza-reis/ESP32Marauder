#pragma once

#ifndef EvilPortal_h
#define EvilPortal_h

#include "LinkedList.h"

#include "settings.h"

#include "lang_var.h"

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