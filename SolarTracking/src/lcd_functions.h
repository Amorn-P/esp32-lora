// --- lcd_functions.h (CLEANED) --- 
#ifndef LCD_FUNCTIONS_H 
#define LCD_FUNCTIONS_H 
#include <LiquidCrystal_I2C.h> 
#include <Wire.h> 

extern LiquidCrystal_I2C lcd; 

void initLCD(); 
void showLCDMessage(const char* line1, const char* line2); 
void showFourLineMessage(const char* line1, const char* line2, const char* line3, const char* line4); 
#endif