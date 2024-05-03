#include "configs.h"

#include <WiFi.h>
#include <Wire.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <Arduino.h>

#include "Assets.h"
#include "WiFiScan.h"
#include "Buffer.h"

#include "CommandLine.h"
//#include "lang_var.h"

WiFiScan wifi_scan_obj;
Buffer buffer_obj;
CommandLine cli_obj;

const String PROGMEM version_number = MARAUDER_VERSION;

uint32_t currentTime  = 0;


void setup()
{

  Serial.begin(115200);

  Serial.println("ESP-IDF version is: " + String(esp_get_idf_version()));

  wifi_scan_obj.RunSetup();

  buffer_obj = Buffer();
  
  cli_obj.RunSetup();
  Serial.println(F("CLI Ready"));
}


void loop()
{
  currentTime = millis();
  bool mini = false;
  
  cli_obj.main(currentTime);

  wifi_scan_obj.main(currentTime);

  buffer_obj.save();

  delay(50);
}