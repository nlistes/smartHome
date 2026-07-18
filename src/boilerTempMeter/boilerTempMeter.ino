#include <OneWire.h>
#include <DallasTemperature.h>

// ==== Test options ==================
//#define _APP_TEST_
//#define _WIFI_TEST_
//#define _MQTT_TEST_


// ==== Host parameters ===============
#define SOFTWARE_VERSION "20260718-01"
#define HOSTNAME "Boilera Temperatūra"
#define DEVICE_TYPE "Termometrs"
#define DEVICE_NAME "Boileris"

#include "src/espTask.inc"


#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.70"
#define MQTT_CLIENT_NAME "espTest-"
#define MQTT_USER_NAME "mqtt"
#define MQTT_PASSWORD "mqtt"
#else
#define MQTT_SERVER "10.20.30.80"
#define MQTT_CLIENT_NAME "espTask-"
#define MQTT_USER_NAME "mqtt"
#define MQTT_PASSWORD "mqtt"
#endif // _MQTT_TEST_

#include "src/espHA.inc"

#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 18
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// function to print a device address
void printOneWireAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) _I_PP("0");
		_I_PH(deviceAddress[i]);
	}
}

#ifdef _APP_TEST_
DeviceAddress topThermometer = { 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }; // 1
DeviceAddress caseThermometer = { 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }; // 08. 28 DA 4A 9C 04 00 00 2C
#else
DeviceAddress topThermometer = { 0x28, 0x50, 0xCE, 0x66, 0x04, 0x00, 0x00, 0xB7 }; // 0
DeviceAddress caseThermometer = { 0x28, 0xFC, 0xCE, 0x66, 0x04, 0x00, 0x00, 0x96 }; // 3
#endif // _APP_TEST_

volatile float topTemperature, caseTemperature;

#define TEMPERATURE_PRECISION 10
#define TEMPERATURE_READ_PERIOD 15

HASensorNumber haTopTemperature("haTopTemperature");
HASensorNumber haCaseTemperature("haCaseTemperature");

void onSendResult()
{
		if ((topTemperature > 0) && (caseTemperature > 0))
		{
			haTopTemperature.setValue(topTemperature);
			_E_PMP("haTopTemperature"); _E_PP(" = "); _E_PL(topTemperature);

			haCaseTemperature.setValue(caseTemperature);
			_E_PMP("haCaseTemperature"); _E_PP(" = "); _E_PL(caseTemperature);
		}
}
Task taskSendResult(TASK_IMMEDIATE, TASK_ONCE, &onSendResult, &ts);

void onShowTemperature()
{
	_I_PMP("Sensor ");  _I_PP("[");  printOneWireAddress(topThermometer); _I_PP("] temp TOP: "); _I_PL(topTemperature);
	_I_PMP("Sensor ");  _I_PP("[");  printOneWireAddress(caseThermometer); _I_PP("] temp CASE: "); _I_PL(caseTemperature);
	taskSendResult.restart();
}
Task taskShowTempereature(TASK_IMMEDIATE, TASK_ONCE, &onShowTemperature, &ts);

void onGetTempereature()
{
	_I_PML("Getting temperatures... ");
#ifndef _TEST_
	topTemperature = sensors.getTempC(topThermometer);
	caseTemperature = sensors.getTempC(caseThermometer);
#else
	topTemperature = 2;
	caseTemperature = 1;
#endif // !_TEST_

	taskShowTempereature.restart();
	_I_PML("DONE");
}
Task taskGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &onGetTempereature, &ts);

void onPrepareTemperature()
{
	_I_PL();  _I_PML("Requesting temperatures... ");
	sensors.requestTemperatures();
	taskGetTempereature.restartDelayed(1 * TASK_SECOND);
	_I_PML("DONE");
}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onPrepareTemperature, &ts);



void setup()
{
#include "src/espTask_setup.inc"

#include "src/espHA_setup.inc"

	sensors.setResolution(topThermometer, TEMPERATURE_PRECISION);
	sensors.setResolution(caseThermometer, TEMPERATURE_PRECISION);
	sensors.setWaitForConversion(false);

	haTopTemperature.setIcon("mdi:coolant-temperature");
	haTopTemperature.setName("Augša");
	haTopTemperature.setUnitOfMeasurement("°C");

	haCaseTemperature.setIcon("mdi:coolant-temperature");
	haCaseTemperature.setName("Vidus");
	haCaseTemperature.setUnitOfMeasurement("°C");

	taskPrepareTempereature.enableDelayed();

} // setup()

void loop()
{
	ts.execute();
}
