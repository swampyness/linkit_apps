#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LBattery.h>
#include <LGPS.h>
#include <wrapper_linkit_gps.h>
#include <wrapper_linkit_lcd.h>
#include <wrapper_linkit_tacho.h>

LinkItTacho tacho1;
LinkItLCD lcd1;
LinkItGPS gps1;

void setup() {
	tacho1.init();
	gps1.init();
	lcd1.init();
	//lcd1.charset();
}

void display() {

	lcd1.print(0, 1, (String)(tacho1.kph) + "kph");
	lcd1.print(0, 0, (String)((int)tacho1.kph));
	lcd1.print(2, 0, "kph");

	if (LBattery.isCharging()) lcd1.icon(0, 3, ICON_CHARGING);

	lcd1.icon(1, 3, ICON_BATTERY_FULL);
	lcd1.print(2, 3, (String)LBattery.level());
	lcd1.print(5, 3, "%");

	if (gps1.fix != 0) {
		lcd1.print(16, 3, (String)(char)CHAR_STAR_SUPER);
		lcd1.print(11, 0, (String)gps1.view[S_TIME]);
		lcd1.icon(19, 0, ICON_AM + gps1.pm);
	} else {
		if (random(2) == 0) {
			lcd1.print(16, 3, (String)(char)CHAR_SQUARE_SUPER);
		} else {
			lcd1.print(16, 3, (String)(char)CHAR_STAR_SUPER);
		}
	}

	lcd1.icon(17, 3, ICON_SAT);
	lcd1.print(18, 3, gps1.view[S_SAT]);
}

void loop() {
	gps1.update();
	if (strcmp(gps1.view[S_TIME], gps1.view[S_TIME_LAST])) display();
}

