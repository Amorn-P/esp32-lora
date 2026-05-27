#ifndef SOCIAL_HANDLERS_H
#define SOCIAL_HANDLERS_H

#include <Arduino.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <UniversalTelegramBot.h>

extern WebServer server;
extern char Bot_Token_1[60];
extern char Bot_Group[20];
extern int BlynkRst_Hr_int;
extern int BlynkRst_Min_int;
extern UniversalTelegramBot bot1;
extern WiFiClientSecure secured_client1;

/**
 * Builds the system status message for Telegram
 */
void Message_telegram() {
    if (strlen(Telegram_Device_Name_Param.valueBuffer) > 0) {
        message = String("Device Name: ") + String(Telegram_Device_Name_Param.valueBuffer) + String("\n");
    } else {
        message = "";
    }
    message += String("\n") + st2;
    message += String("\n") + st50;
    message += String("\n") + st11;
    message += String("\n") + st50;
    message += String("\n") + st21 + SSID_NAME;
    message += String("\n") + st22 + PASSWORD_NAME;
    message += String("\n") + st50;
    message += String("\n") + st13 + Sel_SelIP_Sys;
    message += String("\n") + st23 + IP;
    message += String("\n") + st24 + Sub_M;
    message += String("\n") + st25 + Gate_way;
    message += String("\n") + st50;
    message += String("\n") + st30 + String("\n") + WiFi.macAddress();
    message += String("\n") + st50;
    message += String("\n") + st32;
    
    bool resetDisabled = (BlynkRst_Hr_int == -1 || BlynkRst_Hr_int == 99);
    if (resetDisabled) {
        message += String("\n") + st33 + String("Continuous Operation");
    } else {
        message += String("\n") + st33 + String(BlynkRst_Hr_int) + st34 + String(BlynkRst_Min_int) + st35;
    }
    message += String("\n") + st50;
}

// --- WEB HANDLERS ---

void handleTelegramSettings() {
    EEPROM.get(0, Bot_Token_1);
    EEPROM.get(70, Bot_Group);
    EEPROM.get(100, BlynkRst_Hr_int);
    EEPROM.get(110, BlynkRst_Min_int);

    String s = F("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8' name='viewport' content='width=device-width, maximum-scale=1.8'/>");
    s += F("<title>Social Settings</title></head><body style='background-color:#111316;font-family:verdana;color:white;padding:20px;'>");
    s += F("<div style='max-width:600px;margin:auto;background:#032E3B;padding:20px;border-radius:10px;'>");
    s += F("<h2 style='text-align:center;color:#80deea;'>Social & Reset Config</h2>");

    // Telegram Form
    s += F("<form action='/save-telegram' method='POST' style='background:#1a1a1a;padding:15px;border-radius:5px;margin-bottom:20px;'>");
    s += F("<h3 style='color:#0088cc;'>📱 Telegram Bot</h3>");
    s += F("Token:<br><input type='text' name='bot_token' value='");
    s += String(Bot_Token_1);
    s += F("' style='width:90%;'><br>");
    s += F("Group ID:<br><input type='text' name='group_id' value='");
    s += String(Bot_Group);
    s += F("' style='width:90%;'><br><br>");
    s += F("<button type='submit' style='background:#0088cc;color:white;border:none;padding:10px 20px;border-radius:5px;'>Save Telegram</button></form>");

    // Reset Form
    s += F("<form action='/save-reset' method='POST' style='background:#1a1a1a;padding:15px;border-radius:5px;'>");
    s += F("<h3 style='color:#ff9800;'>🔄 System Reset</h3>");
    s += F("Hour (0-23):<br><input type='number' name='reset_hour' value='");
    s += String(BlynkRst_Hr_int);
    s += F("' style='width:50px;'><br>");
    s += F("Minute (0-59):<br><input type='number' name='reset_minute' value='");
    s += String(BlynkRst_Min_int);
    s += F("' style='width:50px;'><br><br>");
    s += F("<button type='submit' style='background:#ff9800;color:white;border:none;padding:10px 20px;border-radius:5px;'>Save Reset Time</button></form>");

    s += F("<br><a href='/' style='color:#80deea;'>Back to Home</a></div></body></html>");
    server.send(200, "text/html", s);
}

void handleTelegramSave() {
    if (server.hasArg("bot_token") && server.hasArg("group_id")) {
        String token = server.arg("bot_token");
        String group = server.arg("group_id");
        token.toCharArray(Bot_Token_1, 60);
        group.toCharArray(Bot_Group, 20);
        EEPROM.put(0, Bot_Token_1);
        EEPROM.put(70, Bot_Group);
        EEPROM.commit();
        bot1 = UniversalTelegramBot(String(Bot_Token_1), secured_client1);
        server.send(200, "text/html", "OK - Telegram Saved. <a href='/telegram-settings'>Back</a>");
    }
}

void handleResetSave() {
    if (server.hasArg("reset_hour") && server.hasArg("reset_minute")) {
        BlynkRst_Hr_int = server.arg("reset_hour").toInt();
        BlynkRst_Min_int = server.arg("reset_minute").toInt();
        EEPROM.put(100, BlynkRst_Hr_int);
        EEPROM.put(110, BlynkRst_Min_int);
        EEPROM.commit();
        server.send(200, "text/html", "OK - Reset Time Saved. <a href='/telegram-settings'>Back</a>");
    }
}

#endif