/**
   EEPROM Data Conversion and Loading
   Refer to README.md for EEPROM map.
*/
void Converse_value() {
  EEPROM.get(0, Bot_Token_1);
  Serial.print("Bot_Token_1 :");
  Serial.println(Bot_Token_1);
  
  EEPROM.get(70, Bot_Group);
  Serial.print("Bot_Group :");
  Serial.println(Bot_Group);

  BlynkRst_Hr_int = EEPROM.get(100, BlynkRst_Hr_int);
  Serial.print("BlynkRst_Hr :");
  Serial.println(BlynkRst_Hr_int);
  
  BlynkRst_Min_int = EEPROM.get(110, BlynkRst_Min_int);
  Serial.print("BlynkRst_Min :");
  Serial.println(BlynkRst_Min_int);

  Period1_Time_Start_Hr_int = EEPROM.get(150, Period1_Time_Start_Hr_int);
  Serial.print("P1 Start Hr: ");
  Serial.println(Period1_Time_Start_Hr_int);
  
  Period1_Time_Start_Min_int = EEPROM.get(160, Period1_Time_Start_Min_int);
  Serial.print("P1 Start Min: ");
  Serial.println(Period1_Time_Start_Min_int);
  
  Period1_Timeduration_Min_int = EEPROM.get(200, Period1_Timeduration_Min_int);
  Serial.print("P1 Duration: ");
  Serial.println(Period1_Timeduration_Min_int);

  Period2_Time_Start_Hr_int = EEPROM.get(170, Period2_Time_Start_Hr_int);
  Serial.print("P2 Start Hr: ");
  Serial.println(Period2_Time_Start_Hr_int);
  
  Period2_Time_Start_Min_int = EEPROM.get(180, Period2_Time_Start_Min_int);
  Serial.print("P2 Start Min: ");
  Serial.println(Period2_Time_Start_Min_int);
  
  Period2_Timeduration_Min_int = EEPROM.get(210, Period2_Timeduration_Min_int);
  Serial.print("P2 Duration: ");
  Serial.println(Period2_Timeduration_Min_int);

  Manual_Timeduration_Min_int = EEPROM.get(220, Manual_Timeduration_Min_int);
  Serial.print("Manual Duration: ");
  Serial.println(Manual_Timeduration_Min_int);

  Sys_Time_Select = EEPROM.get(230, Sys_Time_Select);
  Serial.print("Time Source (0=RTC 1=NTP): ");
  Serial.println(Sys_Time_Select);

  Period1 = EEPROM.get(240, Period1);
  Serial.print("Period1 Status: ");
  Serial.println(Period1);
  
  Period2 = EEPROM.get(250, Period2);
  Serial.print("Period2 Status: ");
  Serial.println(Period2);
  
  Manual = EEPROM.get(260, Manual);
  Serial.print("Manual Status: ");
  Serial.println(Manual);

  prevPeriod1 = Period1;
  prevPeriod2 = Period2;
  prevManual = Manual;

  currentMode = EEPROM.get(500, currentMode);
  Serial.print("WiFi Mode (0=AP 1=Fast): ");
  Serial.println(currentMode);

  strcpy(ipAddressValue, ipAddressParam.valueBuffer);        // Static IP Address
  strcpy(gatewayValue, gatewayParam.valueBuffer);            // Gateway Address
  strcpy(netmaskValue, netmaskParam.valueBuffer);            // Subnet Mask
  strcpy(primaryDNSValue, primaryDNSParam.valueBuffer);      // Primary DNS
  strcpy(secondaryDNSValue, secondaryDNSParam.valueBuffer);  // Secondary DNS
  
  // Blynk Configuration
  strcpy(Blynk_Token_1, Blynk_Token_11.valueBuffer);        // Blynk Token
  strcpy(configblynk, configblynkserver11.valueBuffer);      // Blynk Server Address

  if (String(Chkbox_SelIP_Sys11.valueBuffer) == "selected") {
    Sel_SelIP_Sys = "Static IP";
    Sel_1_SelIP_Sys = 1;
  } else {
    Sel_SelIP_Sys = "Dynamic IP";
    Sel_1_SelIP_Sys = 0;
  }
  
  if (String(ChkboxSelBlynk_11.valueBuffer) == "selected") {
    Serial.println(" Mode Blynk");
    Sel_Blynk_Mode = "Blynk ";
    Sel_1_Blynk_Mode = 1;  // (1=, 0=)
  } else {
    // Mode Local ( Blynk)
    Sel_Blynk_Mode = "Blynk ";
    Sel_1_Blynk_Mode = 0;
  }
}
