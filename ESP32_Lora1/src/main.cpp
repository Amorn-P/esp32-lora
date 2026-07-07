/**
 * main.cpp - ESP32_Lora Unified Firmware
 * 
 * Master (BOARD_TYPE=0): CommCore + LoRa Gateway + LCD2004
 * Slave  (BOARD_TYPE=1): LoRa + RTC + Relays (autonomous)
 */
 
#include <Arduino.h>
#include <esp_task_wdt.h>

// ============================================================
// MASTER INCLUDES
// ============================================================
#if BOARD_TYPE == 0
  #include <LiquidCrystal_I2C.h>
  #include "CommCore.h"
  #include "12_List_Wf.h"
  #include "01_Social_Handlers.h"
  #include "00_Blynk_App.h"
#endif

// ============================================================
// COMMON INCLUDES
// ============================================================
#include "User_config.h"
#include "Hardware_Map.h"
#include "Hardware_Service.h"
#include "Time_Manager.h"
#include "Lora_Protocol.h"
#include "Lora_Service.h"
#include "Scheduler.h"

// ============================================================
// GLOBALS
// ============================================================
uint8_t myID = 0;

#if BOARD_TYPE == 0
  LiquidCrystal_I2C lcd(0x27, 20, 4);

  // Slave status tracking (for Master)
  struct SlaveInfo {
    uint8_t relayStatus;
    uint8_t currentMode;
    uint32_t lastSeen;
    bool online;
  } slaves[8];

  // Schedule ACK tracking
  bool scheduleAcked[8] = {false};

  unsigned long lastLCDUpdate = 0;
  unsigned long lastTimeSync = 0;
  unsigned long lastOfflineCheck = 0;

  // Blynk LED pin mapping per slave board
  // Maps slave relay bits to Blynk virtual pins
  static const uint8_t slaveLEDBase[8] = {
    0,   // 0: Master (unused)
    61,  // 1: B1 → V61
    62,  // 2: B2 → V62
    63,  // 3: B3 → V63
    66,  // 4: B4 → V66-V69
    70,  // 5: B5 → V70-V75
    76,  // 6: B6 → V76-V79
    81   // 7: B7 → V81-V86
  };

  // Number of relay LEDs per slave
  static const uint8_t slaveLEDCount[8] = {
    0, 1, 1, 1, 4, 6, 4, 6
  };

  // LoRa online status LEDs (V150 = B1, V156 = B7)
  static const uint8_t loraOnlinePin[8] = {0, 150, 151, 152, 153, 154, 155, 156};
#endif

unsigned long lastHeartbeat = 0;
unsigned long lastSchedulerRun = 0;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================
#if BOARD_TYPE == 0
  void handleLoRaMaster();
  void updateLCD();
  void broadcastSchedule();
  void broadcastStop();
  void broadcastManual(uint8_t relayID, uint16_t durationMin);
  void broadcastTimeSync();
  void checkOfflineSlaves();
  uint8_t relayToBoard(uint8_t globalRelay);

  // Load IotWebConf params from EEPROM to runtime globals
  void Converse_value() {
    extern IotWebConfTextParameter ipAddressParam, gatewayParam, netmaskParam;
    extern IotWebConfTextParameter primaryDNSParam, secondaryDNSParam;
    extern IotWebConfTextParameter Blynk_Token_11, configblynkserver11;
    extern IotWebConfCheckboxParameter Chkbox_SelIP_Sys11, ChkboxSelBlynk_11;
    extern char ipAddressValue[128], gatewayValue[128], netmaskValue[128];
    extern char primaryDNSValue[128], secondaryDNSValue[128];

    EEPROM.get(0, Bot_Token_1);
    EEPROM.get(70, Bot_Group);

    strcpy(ipAddressValue, ipAddressParam.valueBuffer);
    strcpy(gatewayValue, gatewayParam.valueBuffer);
    strcpy(netmaskValue, netmaskParam.valueBuffer);
    strcpy(primaryDNSValue, primaryDNSParam.valueBuffer);
    strcpy(secondaryDNSValue, secondaryDNSParam.valueBuffer);
    strcpy(Blynk_Token_1, Blynk_Token_11.valueBuffer);
    strcpy(configblynk, configblynkserver11.valueBuffer);

    if (String(Chkbox_SelIP_Sys11.valueBuffer) == "selected") {
      Sel_SelIP_Sys = "Static IP"; Sel_1_SelIP_Sys = 1;
    } else {
      Sel_SelIP_Sys = "Dynamic IP"; Sel_1_SelIP_Sys = 0;
    }
    if (String(ChkboxSelBlynk_11.valueBuffer) == "selected") {
      Sel_Blynk_Mode = "Blynk ON"; Sel_1_Blynk_Mode = 1;
    } else {
      Sel_Blynk_Mode = "Blynk OFF"; Sel_1_Blynk_Mode = 0;
    }
  }
#else
  void handleLoRaSlave();
  void sendHeartbeat();
#endif

// ============================================================
// MASTER HELPERS
// ============================================================
#if BOARD_TYPE == 0

uint8_t relayToBoard(uint8_t globalRelay) {
    if (globalRelay >= 1 && globalRelay <= 4)  return 4;
    if (globalRelay >= 5 && globalRelay <= 10) return 5;
    if (globalRelay >= 11 && globalRelay <= 14) return 6;
    if (globalRelay >= 15 && globalRelay <= 20) return 7;
    return 0;
}

// Send manual command: valve relay ON + B3 pump ON
void broadcastManual(uint8_t relayID, uint16_t durationMin) {
    uint8_t valveBoard = relayToBoard(relayID);
    if (valveBoard == 0) {
        Serial.printf("[Manual] Invalid relay ID: %d\n", relayID);
        return;
    }

    LuckyPacket p;
    memset(&p, 0, sizeof(LuckyPacket));
    p.senderId    = 0;
    p.cmdType     = CMD_MANUAL;
    p.activeRelay = relayID;
    p.duration1   = durationMin;

    // 1. Valve board
    p.targetId = valveBoard;
    LoraService::sendPacket(p);
    Serial.printf("[Manual] Board %d, Relay %d, %d min\n", valveBoard, relayID, durationMin);

    // 2. B3 pump (auto with manual)
    p.targetId = 3;
    p.activeRelay = 0; // B3 doesn't care which relay, just turns on
    LoraService::sendPacket(p);
    Serial.printf("[Manual] B3 pump ON, %d min\n", durationMin);
}

#endif // BOARD_TYPE == 0

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  // 1. Hardware init (dipswitch, relays, I2C)
  HardwareService::init();
  myID = HardwareService::getBoardID();
  Serial.printf("\n--- ESP32_Lora Board ID: %d (%s) ---\n",
                myID, (BOARD_TYPE == 0) ? "MASTER" : "SLAVE");

  // 2. RTC init
  TimeManager::init();

  // 3. LoRa init
  LoraService::init();

  // 4. Watchdog (10 second timeout)
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);

#if BOARD_TYPE == 0
  // === MASTER SETUP ===

  // 5. LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Lucky-Lora AA");

  // 6. CommCore init (manual, no CommCore.cpp)
  pinMode(LED_INTERNET, OUTPUT);
  digitalWrite(LED_INTERNET, 0);
  static_ip();
  Converse_value();
  Iotwencof_start();
  needReset = false;
  CommCore_telegramSetup();
  if (Sel_1_Blynk_Mode == 1) {
    Blynk.config(Blynk_Token_1, configblynk, 8080);
  }
  timer.setInterval(2000L, internetcheck);

  // 7. Scheduler init (for broadcast)
  Scheduler::init(0);

  // 8. FreeRTOS tasks
  xTaskCreatePinnedToCore(Task1_CommCore, "T1_IoT",  8192, NULL, 1, &CommTask1, 0);
  xTaskCreatePinnedToCore(Task4_CommCore, "T4_Blynk", 8192, NULL, 1, &CommTask4, 1);

  // 9. Init offline tracking
  for (int i = 1; i < 8; i++) {
    slaves[i].online = false;
    slaves[i].lastSeen = 0;
    scheduleAcked[i] = false;
  }

#else
  // === SLAVE SETUP ===

  // 5. Scheduler init (load schedule from NVM)
  Scheduler::init(myID);

#endif

  Serial.println("Setup complete.");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  esp_task_wdt_reset();

#if BOARD_TYPE == 0
  // MASTER LOOP
  timer.run();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  handleLoRaMaster();

  // LCD update every 2s
  if (millis() - lastLCDUpdate > 2000) {
    updateLCD();
    lastLCDUpdate = millis();
  }

  // Time sync broadcast every 60 minutes
  if (millis() - lastTimeSync > 3600000) {
    broadcastTimeSync();
    lastTimeSync = millis();
  }

  // Offline check every 30s
  if (millis() - lastOfflineCheck > 30000) {
    checkOfflineSlaves();
    lastOfflineCheck = millis();
  }

#else
  // SLAVE LOOP

  // Heartbeat every 10s
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }

  // Handle incoming LoRa commands
  handleLoRaSlave();

  // Run scheduler every 1s
  if (millis() - lastSchedulerRun > 1000) {
    Scheduler::run();
    lastSchedulerRun = millis();
  }

#endif
}

// ============================================================
// MASTER: LoRa Handler
// ============================================================
#if BOARD_TYPE == 0

void handleLoRaMaster() {
  LuckyPacket packet;
  if (!LoraService::receivePacket(packet)) return;

  uint8_t sid = packet.senderId;
  if (sid < 1 || sid > 7) return;

  switch (packet.cmdType) {

    case CMD_HEARTBEAT:
      if (packet.targetId == 0) {
        slaves[sid].relayStatus = packet.relayStatus;
        slaves[sid].currentMode = packet.currentMode;
        slaves[sid].lastSeen = millis();
        if (!slaves[sid].online) {
          slaves[sid].online = true;
          Serial.printf("[LoRa] B%d online\n", sid);
        }

        // Update Blynk relay LEDs per slave
        uint8_t basePin = slaveLEDBase[sid];
        uint8_t count   = slaveLEDCount[sid];
        for (uint8_t i = 0; i < count; i++) {
          bool relayOn = (packet.relayStatus & (1 << i)) != 0;
          BLYNK_WRITE_SAFE(basePin + i, relayOn ? 255 : 0);
        }

        // Update LoRa online LED
        BLYNK_WRITE_SAFE(loraOnlinePin[sid], 255);
      }
      break;

    case CMD_ACK:
      scheduleAcked[sid] = true;
      Serial.printf("[LoRa] B%d ACK schedule\n", sid);
      break;

    default:
      break;
  }
}

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print(TimeManager::getFormattedTime().substring(11)); // HH:MM:SS

  lcd.setCursor(0, 2);
  int active = 0;
  for (int i = 1; i < 8; i++) {
    if (slaves[i].online && (millis() - slaves[i].lastSeen < 30000)) {
      active++;
    }
  }
  lcd.printf("Online: %d/7", active);

  lcd.setCursor(0, 3);
  lcd.printf("IP:%s", WiFi.localIP().toString().c_str());
}

void checkOfflineSlaves() {
  for (int i = 1; i < 8; i++) {
    if (slaves[i].online && (millis() - slaves[i].lastSeen > 30000)) {
      slaves[i].online = false;
      scheduleAcked[i] = false;
      Serial.printf("[LoRa] B%d offline (>30s)\n", i);
      BLYNK_WRITE_SAFE(loraOnlinePin[i], 0);
    }
  }
}

// ============================================================
// BROADCAST: Targeted schedule packets
//   B1: Hardcoded 09:00-17:00 (no broadcast needed)
//   B2 (targetId=2): computed start, 50 ON / 10 OFF
//   B3-B7 (targetId=255): Round 1 + Round 2
// ============================================================
void broadcastSchedule() {
    // NOTE: B1 uses hardcoded schedule — no broadcast needed

    // ---- B2: Computed start after Round 1 ends ----
    // Round 1 total = 20 * dur1 - 19 minutes
    // B2 start = next 15-min boundary after Round 1 ends
    // B2 stop  = 13:50 (10 min before Round 2 at 14:00)
    {
        // Access Blynk globals from 00_Blynk_App.h
        extern uint8_t  blynkP1StartHr, blynkP1StartMin;
        extern uint16_t blynkP1Dur;

        uint16_t p1StartMins = blynkP1StartHr * 60 + blynkP1StartMin;
        uint16_t round1Total = 20 * blynkP1Dur - 19;
        uint16_t round1End   = p1StartMins + round1Total;

        // Ceil to next 15-min boundary
        uint16_t b2Start = ((round1End + 14) / 15) * 15;

        // Reset ACK tracking
        scheduleAcked[2] = false;

        LuckyPacket p;
        memset(&p, 0, sizeof(LuckyPacket));
        p.senderId   = 0;
        p.targetId   = 2;
        p.cmdType    = CMD_SET_SCHEDULE;
        p.startHr1   = b2Start / 60;
        p.startMin1  = b2Start % 60;
        p.duration1  = 50;   // ON minutes
        p.startHr2   = PUMP_OFF_MIN; // 10 (OFF minutes)

        LoraService::sendPacket(p);
        Serial.printf("[LoRa] B2 schedule: %02d:%02d (R1 ends %02d:%02d)\n",
                      b2Start/60, b2Start%60, round1End/60, round1End%60);
    }

    // ---- B3-B7 Group: Round 1 + Round 2 ----
    {
        extern uint8_t  blynkP1StartHr, blynkP1StartMin, blynkP2StartHr, blynkP2StartMin;
        extern uint16_t blynkP1Dur, blynkP2Dur;

        // Reset ACK tracking for B3-B7
        for (int i = 3; i <= 7; i++) scheduleAcked[i] = false;

        LuckyPacket p;
        memset(&p, 0, sizeof(LuckyPacket));
        p.senderId   = 0;
        p.targetId   = 255;  // All B3-B7
        p.cmdType    = CMD_SET_SCHEDULE;
        p.startHr1   = blynkP1StartHr;
        p.startMin1  = blynkP1StartMin;
        p.duration1  = blynkP1Dur;
        p.startHr2   = blynkP2StartHr;
        p.startMin2  = blynkP2StartMin;
        p.duration2  = blynkP2Dur;

        LoraService::sendPacket(p);
        Serial.printf("[LoRa] B3-B7 schedule: R1 %02d:%02d/%dmin, R2 %02d:%02d/%dmin\n",
                      blynkP1StartHr, blynkP1StartMin, blynkP1Dur,
                      blynkP2StartHr, blynkP2StartMin, blynkP2Dur);
    }
}

// Time sync broadcast (epoch packed into schedule fields)
void broadcastTimeSync() {
    LuckyPacket p;
    memset(&p, 0, sizeof(LuckyPacket));
    p.senderId  = 0;
    p.targetId  = 255;
    p.cmdType   = CMD_TIME_SYNC;

    uint32_t epoch = TimeManager::now().unixtime();
    p.startHr1  = (epoch >> 24) & 0xFF;
    p.startMin1 = (epoch >> 16) & 0xFF;
    p.duration1 = (epoch >> 8)  & 0xFF;
    p.startHr2  = epoch & 0xFF;

    LoraService::sendPacket(p);
    Serial.printf("[LoRa] Time sync: %u\n", epoch);
}

// Emergency stop broadcast
void broadcastStop() {
  LuckyPacket p;
  memset(&p, 0, sizeof(LuckyPacket));
  p.cmdType  = CMD_STOP_ALL;
  p.targetId = 255;

  if (LoraService::sendPacket(p)) {
    Serial.println("[LoRa] STOP ALL broadcast OK");
  }
}

#endif // BOARD_TYPE == 0

// ============================================================
// SLAVE: LoRa Handler
// ============================================================
#if BOARD_TYPE != 0

void handleLoRaSlave() {
  LuckyPacket packet;
  if (!LoraService::receivePacket(packet)) return;

  // Only process packets addressed to us or broadcast
  if (packet.targetId != myID && packet.targetId != 255) return;

  switch (packet.cmdType) {
    case CMD_MANUAL:
      Scheduler::setManual(packet.activeRelay, packet.duration1);
      Serial.printf("[LoRa] Manual: relay %d, %d min\n", packet.activeRelay, packet.duration1);
      break;

    case CMD_STOP_ALL:
      Scheduler::stopAll();
      Serial.println("[LoRa] STOP ALL — persistent");
      break;

    case CMD_SET_SCHEDULE: {
      Schedule s1 = {packet.startHr1, packet.startMin1, packet.duration1};
      Schedule s2 = {packet.startHr2, packet.startMin2, packet.duration2};
      Scheduler::setSchedule(s1, s2);
      Serial.printf("[LoRa] Schedule: R1 %02d:%02d/%dmin, R2 %02d:%02d/%dmin\n",
                    s1.startHr, s1.startMin, s1.duration,
                    s2.startHr, s2.startMin, s2.duration);

      // Send ACK back to Master
      LuckyPacket ack;
      memset(&ack, 0, sizeof(LuckyPacket));
      ack.senderId = myID;
      ack.targetId = 0;
      ack.cmdType  = CMD_ACK;
      LoraService::sendPacket(ack);
      break;
    }

    case CMD_TIME_SYNC: {
      // Reconstruct epoch from schedule fields
      uint32_t epoch = ((uint32_t)packet.startHr1 << 24) |
                       ((uint32_t)packet.startMin1 << 16) |
                       ((uint32_t)packet.duration1 << 8) |
                       (uint32_t)packet.startHr2;
      TimeManager::setEpoch(epoch);
      Serial.printf("[LoRa] Time sync: %u\n", epoch);
      break;
    }

    default:
      break;
  }
}

// Send heartbeat to Master
void sendHeartbeat() {
  LuckyPacket packet;
  memset(&packet, 0, sizeof(LuckyPacket));
  packet.senderId    = myID;
  packet.targetId    = 0;
  packet.cmdType     = CMD_HEARTBEAT;
  packet.relayStatus = HardwareService::getRelayStatus();
  packet.currentMode = Scheduler::getCurrentMode();

  LoraService::sendPacket(packet);
}

#endif // BOARD_TYPE != 0
