#include <LiquidCrystal_SR_LCD3.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal_SR_LCD3 lcd(A2, A0, A1);

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = {0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C}, outsideThermometer = { 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA };


int c = 0;


void setup()
{
	Serial.begin(9600);

	lcd.begin(16, 2);        // set up the LCD's number of columns and rows
	lcd.print("Aiziet...");
	lcd.setCursor(0, 1);

	sensors.begin();

	Serial.print("Current resolution: "); Serial.println(sensors.getResolution());
	sensors.setResolution(12);
	Serial.print("Current resolution: "); Serial.println(sensors.getResolution());
	Serial.print("insideThermometer is connected : "); Serial.println(sensors.isConnected(insideThermometer));

	Serial.print("Sensors count: "); Serial.println(sensors.getDS18Count());
	if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
	if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");

	Serial.print("Device 0 Address: ");	printAddress(insideThermometer); Serial.println();
	Serial.print("Device 1 Address: ");	printAddress(outsideThermometer); Serial.println();

}

void loop()
{
	lcd.setCursor(0, 0);
	Serial.println();  Serial.print("Requesting temperatures... ");
	lcd.print("Get temp... ");
	sensors.requestTemperatures(); // Send the command to get temperatures
	Serial.println("DONE");
	lcd.print("DONE");
	lcd.setCursor(0, 0);

	float inTemp = sensors.getTempC(insideThermometer);
		if (inTemp != DEVICE_DISCONNECTED_C)
		{
			Serial.print("Temperature for the insideThermometer ");
			Serial.println(inTemp);
			lcd.print(inTemp);
		}
		else
		{
			Serial.println("Error: Could not read temperature data");
			lcd.print("Error...        ");
		}

		float tempC = sensors.getTempC(outsideThermometer);
		if (tempC != DEVICE_DISCONNECTED_C)
		{
			Serial.print("Temperature for the outsideThermometer ");
			Serial.println(tempC);
			lcd.print(tempC);
		}
		else
		{
			Serial.println("Error: Could not read temperature data");
			lcd.print("Error...        ");
		}

	lcd.setCursor(0, 1);
	c++;
	delay(5000);
	lcd.print(c);

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

