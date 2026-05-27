// ===============================================
// WiFi Access Point  Static IP Configuration
// ===============================================

/**
 *  WiFi Access Point 
 * @param apName  Access Point
 * @param password  Access Point
 * @return true  AP , false 
 */
bool connectAp(const char* apName, const char* password) {
  // WiFi Access Point 
  // 4 
  return WiFi.softAP(apName, password, 4);
}

/**
 *  Static IP Address 
 *  Static IP 
 */
void static_ip() {
  // WiFi 
  // connectAp:  Access Point
  // connectWifi:  WiFi Network
  iotWebConf.setApConnectionHandler(&connectAp);
  iotWebConf.setWifiConnectionHandler(&connectWifi);
  
  // bool connectAp(const char* apName, const char* password);
  void connectWifi(const char* ssid, const char* password);
}
