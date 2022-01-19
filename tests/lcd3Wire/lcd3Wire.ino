#include "LiquidCrystal_SR_LCD3.h"

LiquidCrystal_SR_LCD3 lcd(A2, A0, A1);

int c = 0;


void setup()
{
	lcd.begin(16, 2);        // set up the LCD's number of columns and rows
	lcd.print("Aiziet...");
	lcd.setCursor(0, 1);
}

void loop()
{
	c++;
	lcd.print(c);
	delay(1000);
	lcd.setCursor(0, 1);
}
