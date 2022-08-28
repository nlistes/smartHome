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

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#define COM_SPEED 74880
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#define COM_SPEED 115200
#define ONE_WIRE_BUS 21
#endif

#include <TaskScheduler.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];


#ifdef _TEST_
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_CLIENT_NAME "tempMeter_test"
#else
#define PRIMARY_SSID "PAGRABS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_CLIENT_NAME "tempMeter"
#endif // _TEST_

#define MQTT_SERVER "10.20.30.60"
#define CONNECTION_TIMEOUT 5

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 20

Scheduler runner;
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
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

void OnConnectWiFi();
Task tConnectWiFi(5 * TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &runner);

void OnConnectMQTT();
Task tConnectMQTT(5 * TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &runner);

void GetTempereature();
Task tGetTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &GetTempereature, &runner);

void sendResult();
Task tSendResult(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &sendResult, &runner);

void outputResult();
Task tOutputResult(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &outputResult, &runner);


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
	_PP("IP address: "); _PL(WiFi.localIP());

	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

}

void OnConnectMQTT()
{
	mqttClient.setServer(MQTT_SERVER, 1883);
	if (!mqttClient.connected())
	{
		_PM("Attempting MQTT connection...");
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

void outputResult();
{

}

void setup()
{

	Serial.begin(COM_SPEED);
	delay(1000);
	_PM("Starting application...");

	//sensors.begin();
	sensors.setResolution(topThermometer, TEMPERATURE_PRECISION);
	sensors.setResolution(caseThermometer, TEMPERATURE_PRECISION);

	tConnectWiFi.enable();
	tConnectMQTT.enable();
	tGetTempereature.enable();
	tSendResult.enable();
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

