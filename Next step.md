 1. Reflash B4 + B5 with v1.1.0 (watchdog, ACK, time sync)
    2. Flash AA (Master) for the first time
    3. Configure AA — connect to Lucky_Lora_AA AP, captive portal:
       - WiFi SSID + password
       - Blynk token + server
       - Telegram bot token + group ID
    4. Set up Blynk dashboard — create widgets:
       - V130-V133: Schedule config
       - V134-V137: Manual + STOP
       - V60-V86: Relay status LEDs
       - V150-V156: LoRa online LEDs
       - V19: SYNC button
    5. Press V19 SYNC → schedules broadcast → B4/B5 ACK → relays activate
    6. Wire INA226 on B2 + B3, flash them
    7. Full system — all 7 slaves + AA

    At step 5 you'll see outputs:


    Master serial:  [LoRa] B4 ACK schedule
                    [LoRa] B5 ACK schedule
    B4 serial:      [LoRa] Schedule: R1 09:00/15min, R2 14:00/15min
    B5 serial:      [LoRa] Schedule: R1 09:00/15min, R2 14:00/15min

Update memory before shutdown
 C:\Users\SKY> wsl
    $ cd /mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation/scripts && bash sync-push.sh
    Or one-liner from CMD without entering WSL:
    wsl bash -c "cd /mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation/scripts && bash sync-push.sh"
	
Flash to ESP32
	from .pio/build/esp32doit-devkit-v1/firmware.bin
	C:\Users\SKY\Documents\PlatformIO\Projects\Project_Preperation\CherryOne_ModbusHealth
	pio run --target upload --upload-port COM3
	or
	cd "C:\Users\SKY\Documents\PlatformIO\Projects\Project_Preperation\CherryOne_ModbusHealth"
	pio run --target upload --upload-port COM3

Set monitor baudrate	
C:\Users\SKY>
	pio device monitor --port COM3
	pio device monitor --port COM3 --baud 115200
