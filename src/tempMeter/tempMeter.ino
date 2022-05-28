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
#define _PH(a) SerialD.print(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WiFi.h>
	#define COM_SPEED 74880
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
	#define COM_SPEED 115200
#endif

WiFiClient ethClient;

#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"

#define SECONDARY_SSID "OSIS"
#define SECONDARY_PASS "IBMThinkPad0IBMThinkPad1"


#include <PubSubClient.h>
#define MQTT_SERVER "10.20.30.60"
PubSubClient mqttClient(ethClient);

#include <TaskScheduler.h>

Scheduler runner;

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress topThermometer = { 0x28, 0x50, 0xCE, 0x66, 0x04, 0x00, 0x00, 0xB7 }; // 0
DeviceAddress caseThermometer = { 0x28, 0xFC, 0xCE, 0x66, 0x04, 0x00, 0x00, 0x96 }; // 3

#define TEMPERATURE_PRECISION 12

volatile float topTemperature, caseTemperature, diffTemperature;

void OnConnectMQTT();
Task tConnctMQTT(5 * TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &runner);

void GetTempereature();
#define TEMPERATURE_READ_PERIOD 20
Task tGetTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &GetTempereature, &runner);

void OutputResult();
Task tOutputResult(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OutputResult, &runner);


void OnConnectMQTT()
{
	if (!mqttClient.connected())
	{
		_PM("Attempting MQTT connection...");
		if (mqttClient.connect("flowMeter"))
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

void GetTempereature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");
	topTemperature = sensors.getTempC(topThermometer);
	caseTemperature = sensors.getTempC(caseThermometer);
	diffTemperature = topTemperature - caseTemperature;
	_PM("Sensor ");  _PP("[");  printAddress(topThermometer); _PP("] temp: "); _PL(topTemperature);
	_PM("Sensor ");  _PP("[");  printAddress(caseThermometer); _PP("] temp: "); _PL(caseTemperature);
	_PM("Temperature difference: "); _PL(diffTemperature);
}

void OutputResult()
{
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", topTemperature);
		mqttClient.publish("tempmeter/0/temp/top", msg);
		_PM("tempmeter/0/temp/top = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", caseTemperature);
		mqttClient.publish("tempmeter/0/temp/case", msg);
		_PM("tempmeter/0/temp/case = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", diffTemperature);
		mqttClient.publish("tempmeter/0/temp/diff", msg);
		_PM("tempmeter/0/temp/diff = "); _PL(msg);

	}
}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PM("Starting application...");

	_PL(); _PM("Connecting to "); _PP(PRIMARY_SSID);
	WiFi.mode(WIFI_STA);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		_PP(".");
	}

	_PL(); _PN("CONNECTED!");
	_PP("IP address: "); _PL(WiFi.localIP());

	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	mqttClient.setServer(MQTT_SERVER, 1883);

	//sensors.begin();
	sensors.setResolution(topThermometer, TEMPERATURE_PRECISION);
	sensors.setResolution(caseThermometer, TEMPERATURE_PRECISION);

	tConnctMQTT.enable();
	tGetTempereature.enable();
	tOutputResult.enable();
}

void loop()
{
	mqttClient.loop();
	runner.execute();
}

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

