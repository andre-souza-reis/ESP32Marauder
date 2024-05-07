#include <stdio.h>
#include <Arduino.h>

#include "WiFiScan.h"
#include "CommandLine.h"

#define MARAUDER_VERSION "v0.13.9"

WiFiScan wifi_scan_obj;
CommandLine cli_obj;

const String PROGMEM version_number = MARAUDER_VERSION;

uint32_t currentTime  = 0;


void setup()
{

  Serial.begin(115200);

  Serial.println("ESP-IDF version is: " + String(esp_get_idf_version()));

  wifi_scan_obj.RunSetup();

  cli_obj.RunSetup();
}


void loop()
{
  currentTime = millis();

  cli_obj.main(currentTime);

  wifi_scan_obj.main(currentTime);

  delay(1);
}