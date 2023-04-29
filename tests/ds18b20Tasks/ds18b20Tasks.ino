// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
#define _PN(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PH(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

#if defined(ARDUINO_ARCH_ESP8266)
//#include <ESP8266WiFi.h>
#define COM_SPEED 74880
#elif defined(ARDUINO_ARCH_ESP32)
//#include <WiFi.h>
#define COM_SPEED 115200
#endif


#include <TaskScheduler.h>

#include <DallasTemperature.h>
#include <OneWire.h>

Scheduler runner;

#define TEMPERATURE_READ_PERIOD 20

void prepareTemperature();
Task tPrepareTemperature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &prepareTemperature, &runner);


#define ONE_WIRE_BUS D5
//#define ONE_WIRE_BUS 21
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define TEMPERATURE_PRECISION 12

//DeviceAddress insideThermometer = { 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }
//DeviceAddress outsideThermometer = { 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA };
DeviceAddress ds18b20[20] =
{
	{ 0x28, 0x50, 0xCE, 0x66, 0x04, 0x00, 0x00, 0xB7 }, // 00. 28 50 CE 66 04 00 00 B7
	{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, // 01. 28 88 DC 66 04 00 00 2D
	{ 0x28, 0x0C, 0xF5, 0x66, 0x04, 0x00, 0x00, 0x10 }, // 02. 28 0C F5 66 04 00 00 10
	{ 0x28, 0xFC, 0xCE, 0x66, 0x04, 0x00, 0x00, 0x96 }, // 03. 28 FC CE 66 04 00 00 96
	{ 0x28, 0x02, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xB5 }, // 04. 28 02 D6 66 04 00 00 B5
	{ 0x28, 0x42, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xC0 }, // 05. DELETED
	{ 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }, // 06. 28 E2 E1 66 04 00 00 49
	{ 0x28, 0xAA, 0xF0, 0x66, 0x04, 0x00, 0x00, 0x2C }, // 07. 28 AA F0 66 04 00 00 2C
	{ 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }, // 08. 28 DA 4A 9C 04 00 00 2C
	{ 0x28, 0xFA, 0xA7, 0x97, 0x04, 0x00, 0x00, 0x38 }, // 09. 28 FA A7 97 04 00 00 38
	{ 0x28, 0x2E, 0xE1, 0x66, 0x04, 0x00, 0x00, 0xAB }, // 10. 28 2E E1 66 04 00 00 AB
	{ 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }, // 11. 28 25 DB 66 04 00 00 6A
	{ 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA }, // 12. 28 95 C3 BD 04 00 00 BA
	{ 0x28, 0x03, 0xCB, 0xE6, 0x03, 0x00, 0x00, 0xB1 }, // 13. 28 03 CB E6 03 00 00 B1
	{ 0x28, 0x13, 0x44, 0x67, 0x04, 0x00, 0x00, 0x62 }, // 14. 28 13 44 67 04 00 00 62
	{ 0x28, 0xBB, 0xF4, 0x66, 0x04, 0x00, 0x00, 0x5F }, // 15. 28 BB F4 66 04 00 00 5F
	{ 0x28, 0xA7, 0xE7, 0x66, 0x04, 0x00, 0x00, 0x4B }, // 16. 28 A7 E7 66 04 00 00 4B
	{ 0x28, 0x97, 0x12, 0xEA, 0x03, 0x00, 0x00, 0x63 }, // 17. 28 97 12 EA 03 00 00 63
	{ 0x28, 0x0F, 0xDE, 0x66, 0x04, 0x00, 0x00, 0xC1 }, // 18. 28 0F DE 66 04 00 00 C1
	{ 0x28, 0xDF, 0xA4, 0xEB, 0x03, 0x00, 0x00, 0xEB }  // 19. 28 DF A4 EB 03 00 00 EB
};

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


void setup()
{
	Serial.begin(COM_SPEED);
	delay(100);
	sensors.begin();

	_PL("Version: 1.0");
	_PP("Locating devices... ");
	numberOfDevices = sensors.getDeviceCount();
	_PP("found "); _PP(numberOfDevices); _PL(" devices."); _PL();

	for (int i = 0; i < numberOfDevices; i++)
	{
		if (sensors.getAddress(tempDeviceAddress, i))
		{
			_PP("Found device "); _PP(i); _PP(" with address: "); printAddress(tempDeviceAddress); _PL();
			_PP("Actuall resolution actually is: "); _PL(sensors.getResolution(tempDeviceAddress));
			_PP("Setting resolution to "); _PL(TEMPERATURE_PRECISION); _PL();
			sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
		}
		else
		{
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(" but could not detect address. Check power and cabling");
		}
	}
	tPrepareTemperature.enable();
}

void prepareTemperature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");

	for (int i = 0; i < numberOfDevices; i++)
	{
		if (sensors.getAddress(tempDeviceAddress, i))
		{
			float tempC = sensors.getTempC(tempDeviceAddress);
			//_PM("Sensor "); _PP(i); _PP(" temp: "); _PL(tempC);
			_PM("Sensor ");  _PP(i); _PP(" [");  printAddress(tempDeviceAddress); _PP("] temp: "); _PL(tempC);
		}
	}
	_PN("FINISHED");
}


void loop()
{
	runner.execute();
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
