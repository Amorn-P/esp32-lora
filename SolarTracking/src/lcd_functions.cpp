// --- lcd_functions.cpp --- 
#include "lcd_functions.h" 
#include <Arduino.h> 
#include "config.h" 
#include <LiquidCrystal_I2C.h> 
#include "freertos/semphr.h" 

void initLCD() { 
  lcd.init(); 
  lcd.backlight(); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("System Booting..."); 
} 

void showLCDMessage(const char* line1, const char* line2) { 
  if (stateMutex == NULL) return;
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(50)) == pdTRUE) { 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print(line1); 
    lcd.setCursor(0, 1); 
    lcd.print(line2); 
    xSemaphoreGive(stateMutex); 
  }
} 

void showFourLineMessage(const char* line1, const char* line2, const char* line3, const char* line4) { 
  if (stateMutex == NULL) return;
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(1000)) == pdTRUE) { 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print(line1); 
    lcd.setCursor(0, 1); 
    lcd.print(line2); 
    lcd.setCursor(0, 2); 
    lcd.print(line3); 
    lcd.setCursor(0, 3); 
    lcd.print(line4); 
    xSemaphoreGive(stateMutex); 
  }
}