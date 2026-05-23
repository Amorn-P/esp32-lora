/**
 * 01_Social_Handlers.h - Web Handlers for Telegram + Reset (Master Only)
 * Compiled only for BOARD_TYPE == 0
 */
#if BOARD_TYPE == 0

#ifndef SOCIAL_HANDLERS_H
#define SOCIAL_HANDLERS_H

#include "CommCore.h"

void handleTelegramSettings() {
    String html;
    html.reserve(2000);
    html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
           "<title>Set Parameters</title></head>"
           "<body style='background:#111316;color:white;font-family:verdana;padding:20px;'>"
           "<h2 style='color:#ff9800;'>Telegram & Reset</h2>"
           "<form action='/save-telegram' method='POST'>"
           "<div style='margin:20px 0;'><label>Bot Token:</label><br>"
           "<input name='token' style='width:100%;padding:10px;background:#1e1e2e;color:white;border:1px solid #333;border-radius:5px;' value='"
           + String(Bot_Token_1) + "'></div>"
           "<div style='margin:20px 0;'><label>Chat/Group ID:</label><br>"
           "<input name='group' style='width:100%;padding:10px;background:#1e1e2e;color:white;border:1px solid #333;border-radius:5px;' value='"
           + String(Bot_Group) + "'></div>"
           "<button type='submit' style='background:linear-gradient(#ff9800,#f57c00);border:none;color:white;"
           "padding:10px 30px;border-radius:5px;font-size:16px;'>Save</button>"
           "</form><br><a href='/' style='color:#80deea;'>Back</a></body></html>";
    server.send(200, "text/html", html);
}

void handleTelegramSave() {
    if (server.hasArg("token") && server.arg("token").length() > 0) {
        server.arg("token").toCharArray(Bot_Token_1, 60);
        EEPROM.put(0, Bot_Token_1);
    }
    if (server.hasArg("group") && server.arg("group").length() > 0) {
        server.arg("group").toCharArray(Bot_Group, 20);
        EEPROM.put(70, Bot_Group);
    }
    EEPROM.commit();
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Saved</title></head>"
                  "<body style='background:#111316;color:#a5d6a7;text-align:center;padding:50px;font-family:verdana;'>"
                  "<h2>Saved!</h2><a href='/' style='color:#80deea;'>Back</a></body></html>";
    server.send(200, "text/html", html);
}

void handleResetSave() {
    Preferences preferences;
    preferences.begin("iotwebconf", false);
    preferences.clear();
    preferences.end();
    char empty1[60] = {0};
    char empty2[20] = {0};
    EEPROM.put(0, empty1);
    EEPROM.put(70, empty2);
    EEPROM.commit();
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Reset</title></head>"
                  "<body style='background:#111316;color:#ef9a9a;text-align:center;padding:50px;font-family:verdana;'>"
                  "<h2>Factory Reset - Rebooting...</h2></body>"
                  "<script>setTimeout(function(){location.href='/';},10000);</script></html>";
    server.send(200, "text/html", html);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ESP.restart();
}

#endif // SOCIAL_HANDLERS_H
#endif // BOARD_TYPE == 0
