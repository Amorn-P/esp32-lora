 Wiring is simple — I2C is already on all boards (GPIO21/22).

    INA226 Module       ESP32
      VCC        →     3.3V
      GND        →     GND
      SCL        →     GPIO22  (I2C)
      SDA        →     GPIO21  (I2C)

      Vin+       →     Solar panel (+)
      Vin-       →     Load (+) — pump/battery
	  
Solar (+) ───→ INA226 Vin+ ══shunt══ Vin- ───→ Step-down Vin+ ──→ [DC-DC] ──→ Vout+ 5V ──→ ESP32 5V
                                                                                   Vout- ────→ ESP32 GND ──┐
    Solar (-) ──────────────────────────────────→ Step-down Vin- ────────────────────────────────────────────┘
                                                                                                              │
                                     INA226 VCC (3.3V) ←── ESP32 3.3V                                        │
                                     INA226 GND ──────────────────────────────────────────────────────────────┘
                                     INA226 SDA ←──→ GPIO21
                                     INA226 SCL ←──→ GPIO22
									 
           Pos:  4     3     2     1
          GPIO:  35    34    39    36
          Bit:  bit3  bit2  bit1  bit0  (MSB→LSB)

    ID  Binary  Pos1  Pos2  Pos3  Pos4
    1    0001   ON    OFF   OFF   OFF
    2    0010   OFF   ON    OFF   OFF
    3    0011   ON    ON    OFF   OFF
    4    0100   OFF   OFF   ON    OFF
    5    0101   ON    OFF   ON    OFF
    6    0110   OFF   ON    ON    OFF
    7    0111   ON    ON    ON    OFF
    AA   0000   OFF   OFF   OFF   OFF   (Master)
	
    cd C:\Users\SKY\Documents\PlatformIO\Projects\ESP32_Lora

    pio run -e master -t upload --upload-port COMx     → AA
    pio run -e slave  -t upload --upload-port COMx     → B1-B7
	
