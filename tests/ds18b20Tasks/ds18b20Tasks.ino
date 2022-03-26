// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PX(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PP(a)
#define _PL(a)
#define _PX(a)
#endif

#include <TaskScheduler.h>

#include <DallasTemperature.h>
#include <OneWire.h>

Scheduler runner;

#define TEMPERATURE_READ_PERIOD 20

void prepareTemperature();
Task tPrepareTemperature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &prepareTemperature, &runner);


#define ONE_WIRE_BUS D7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define TEMPERATURE_PRECISION 12

//DeviceAddress insideThermometer = { 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }
//DeviceAddress outsideThermometer = { 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA };

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


void setup()
{
	Serial.begin(74880);
	delay(100);
	sensors.begin();

	_PL("Version: 1.0");
	_PP("Locating devices...");
	numberOfDevices = sensors.getDeviceCount();
	_PP("Found "); _PP(numberOfDevices); _PL(" devices."); _PL();

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
	_PL();  _PM("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PM("DONE");

	for (int i = 0; i < numberOfDevices; i++)
	{
		if (sensors.getAddress(tempDeviceAddress, i))
		{
			float tempC = sensors.getTempC(tempDeviceAddress);
			_PP("Temperature for device: "); _PP(i);
			_PP(" Temp C: "); _PL(tempC);
		}
	}
	_PM("END");
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
