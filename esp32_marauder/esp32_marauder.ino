#include "configs.h"

#include <WiFi.h>
#include "EvilPortal.h"
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

#include "settings.h"
#include "CommandLine.h"
#include "lang_var.h"

WiFiScan wifi_scan_obj;
EvilPortal evil_portal_obj;
Buffer buffer_obj;
Settings settings_obj;
CommandLine cli_obj;

const String PROGMEM version_number = MARAUDER_VERSION;

uint32_t currentTime  = 0;


void setup()
{

  Serial.begin(115200);

  Serial.println("ESP-IDF version is: " + String(esp_get_idf_version()));

  settings_obj.begin();

  wifi_scan_obj.RunSetup();

  buffer_obj = Buffer();

  evil_portal_obj.setup();
  
  Serial.println(F("CLI Ready"));
  cli_obj.RunSetup();
}


void loop()
{
  currentTime = millis();
  bool mini = false;

  cli_obj.main(currentTime);

  wifi_scan_obj.main(currentTime);

  buffer_obj.save();

  settings_obj.main(currentTime);

  delay(50);
}