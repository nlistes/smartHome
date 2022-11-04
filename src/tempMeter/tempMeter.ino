// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_
#define _MQTT_TEST_
#define _WIFI_TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
#define _PN(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PH(a) SerialD.print(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

#if defined (ARDUINO_ARCH_AVR)
#define COM_SPEED 9600
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#define COM_SPEED 74880
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define COM_SPEED 115200
#endif

//#include <stdlib.h>
//#include <stdio.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) _PP("0");
		_PH(deviceAddress[i]);
	}
}

#include <TaskScheduler.h>
Scheduler ts;

#if defined (ARDUINO_ARCH_AVR)
#include <SPI.h>
#include <Ethernet.h>
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
WiFiClient ethClient;
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
WiFiClient ethClient;
#endif

#ifdef _WIFI_TEST_
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#else
#define PRIMARY_SSID "PAGRABS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _WIFI_TEST_

#define CONNECTION_TIMEOUT 5


void OnConnectWiFi()
{
	_PL(); _PM("Connecting to "); _PP(PRIMARY_SSID);
	WiFi.mode(WIFI_STA);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		_PP(".");
}

	_PL(); _PN("CONNECTED!");
	_PM("IP address: "); _PL(WiFi.localIP());

	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

}
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);

#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "flowMeter_test"
#else
#define MQTT_SERVER "10.20.30.71"
#define MQTT_CLIENT_NAME "flowMeter"
#endif // _MQTT_TEST_

#include <PubSubClient.h>
PubSubClient mqttClient(ethClient);

#define MSG_BUFFER_SIZE	10
char msg[MSG_BUFFER_SIZE];

#define TOPIC_BUFFER_SIZE	40
char topic[TOPIC_BUFFER_SIZE];

void OnConnectMQTT()
{
	if (!mqttClient.connected())
	{
		_PL();  _PM("Attempting MQTT connection... ");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
		{
			_PL(" connected!");
			mqttClient.subscribe("inTopic");
		}
		else
		{
			_PM("failed, rc="); _PL(mqttClient.state());
		}

	}

}
Task tConnectMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);


#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 21
#endif

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 20

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


#ifdef _TEST_
	DeviceAddress topThermometer = { 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }; // 1
	DeviceAddress caseThermometer = { 0x28, 0x42, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xC0 }; // 5
#else
	DeviceAddress topThermometer = { 0x28, 0x50, 0xCE, 0x66, 0x04, 0x00, 0x00, 0xB7 }; // 0
	DeviceAddress caseThermometer = { 0x28, 0xFC, 0xCE, 0x66, 0x04, 0x00, 0x00, 0x96 }; // 3
#endif // _TEST_

volatile float topTemperature, caseTemperature, diffTemperature;

void OnPrepareTemperature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");
}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &OnPrepareTemperature, &ts);


void GetTempereature()
{
	topTemperature = sensors.getTempC(topThermometer);
	caseTemperature = sensors.getTempC(caseThermometer);
	diffTemperature = topTemperature - caseTemperature;
	_PM("Sensor ");  _PP("[");  printAddress(topThermometer); _PP("] temp: "); _PL(topTemperature);
	_PM("Sensor ");  _PP("[");  printAddress(caseThermometer); _PP("] temp: "); _PL(caseTemperature);
	_PM("Temperature difference: "); _PL(diffTemperature);
}
Task tGetTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &GetTempereature, &ts);

void sendResult()
{
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", topTemperature);
		mqttClient.publish("tempmeter/boiler/temp/top", msg);
		_PM("tempmeter/boiler/temp/top = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", caseTemperature);
		mqttClient.publish("tempmeter/boiler/temp/case", msg);
		_PM("tempmeter/boiler/temp/case = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", diffTemperature);
		mqttClient.publish("tempmeter/boiler/temp/diff", msg);
		_PM("tempmeter/boiler/temp/diff = "); _PL(msg);

	}
}
Task tSendResult(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &sendResult, &ts);

void outputResult()
{

}
Task tOutputResult(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &outputResult, &ts);


//void outputResult()

void setup()
{
	Serial.begin(COM_SPEED);
	delay(100);
	_PL("Programm started...");

#if defined (ARDUINO_ARCH_AVR)
	Ethernet.begin(MAC_ADDRESS);
#endif

#if defined(ARDUINO_ARCH_ESP8266) || (defined ARDUINO_ARCH_ESP32)
	WiFi.mode(WIFI_STA);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		_PP(".");
	}
#endif

	_PL("Newtwork connected");
	_PP("IP address: ");

#if defined (ARDUINO_ARCH_AVR)
	_PL(Ethernet.localIP());
#endif

#if defined(ARDUINO_ARCH_ESP8266) || (defined ARDUINO_ARCH_ESP32)
	_PL(WiFi.localIP());
#endif

	_PL("");


	tConnectWiFi.enable();
	tConnectMQTT.enable();
	tGetTempereature.enable();
	tSendResult.enable();
}

void loop()
{
	mqttClient.loop();
	ts.execute();
}
