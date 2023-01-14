// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_
//#define _MQTT_TEST_
#define _WIFI_TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PH(a) SerialD.print(a, HEX)
#define _PMP(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
#define _PML(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#else
#define _PP(a)
#define _PL(a)
#define _PH(a)
#define _PMP(a)
#define _PML(a)
#endif

#include <OneWire.h>
#include <DallasTemperature.h>

// function to print a device address
void printOneWireAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) _PP("0");
		_PH(deviceAddress[i]);
	}
}

#if defined(ARDUINO_ARCH_ESP8266)
#define COM_SPEED 74880
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define COM_SPEED 115200
#endif

//#include <stdlib.h>
//#include <stdio.h>

#include <TaskScheduler.h>
Scheduler ts;

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

#define CONNECTION_TIMEOUT 10

void OnConnectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		_PL(""); _PMP("Connecting to "); _PP(PRIMARY_SSID);
		WiFi.mode(WIFI_STA);
		WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);

		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			_PP(".");
		}

		_PL(); _PML("WIFI CONNECTED!");
		_PMP("IP address: "); _PL(WiFi.localIP());
		//_PL(WiFi.localIP().toString);

		// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
		WiFi.setAutoReconnect(true);
		WiFi.persistent(true);
	}
}
Task tConnectWiFi(CONNECTION_TIMEOUT * TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);


#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "boilerControl_test"
#else
#define MQTT_SERVER "10.20.30.71"
#define MQTT_CLIENT_NAME "boilerControl"
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
		_PL();  _PMP("Connecting to MQTT ["); _PP(MQTT_SERVER); _PL("]");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
		{
			_PML("MQTT CONNECTED!");
			//mqttClient.subscribe("inTopic");
		}
		else
		{
			_PMP("FAILED!!! rc="); _PL(mqttClient.state());
		}
	}

}
Task tConnectMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnConnectMQTT, &ts);

void OnRunMQTT()
{
	mqttClient.loop();
}
Task tRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &OnRunMQTT, &ts);

#ifdef _TEST_
void OnSendTest()
{
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "test");
		snprintf(topic, TOPIC_BUFFER_SIZE, "test/test");
		mqttClient.publish(topic, msg);
		_PMP(topic); _PP(" = ");  _PL(msg);
	}
}
Task tSendTest(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnSendTest, &ts);
#endif // _TEST_

#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 23
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&oneWire);

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 5


#define MAX_BOILER_TEMPERATURE_SENSORS 3
#define ACTUAL_BOILER_TEMPERATURE_SENSORS 2

DeviceAddress boilerThermometer[MAX_BOILER_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, //  1
	{ 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }, //  8
	{ 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA }  // 12
};
volatile float boilerTemperature[MAX_BOILER_TEMPERATURE_SENSORS];


#define MAX_HEAT_TEMPERATURE_SENSORS 2
#define ACTUAL_HEAT_TEMPERATURE_SENSORS 2

DeviceAddress heatThermometer[MAX_HEAT_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA }, // 12
	{ 0x28, 0x13, 0x44, 0x67, 0x04, 0x00, 0x00, 0x62 }  // 14
};
volatile float heatTemperature[MAX_HEAT_TEMPERATURE_SENSORS];

void OnShowTemperature()
{
	for (int i = 0; i < ACTUAL_BOILER_TEMPERATURE_SENSORS; i++)
	{
		_PMP("Boiler temperature [");  _PP(i); _PP("] = "); _PL(boilerTemperature[i]);
	}

	for (int i = 0; i < ACTUAL_HEAT_TEMPERATURE_SENSORS; i++)
	{
		_PMP("Heat temperature [");  _PP(i); _PP("] = "); _PL(heatTemperature[i]);
	}
}
Task taskShowTempereature(TASK_IMMEDIATE, TASK_ONCE, &OnShowTemperature, &ts);

void OnSendTemperature()
{
	if (mqttClient.connected())
	{
		for (uint8_t i = 0; i < ACTUAL_BOILER_TEMPERATURE_SENSORS; i++)
			if (boilerTemperature[i] > 0)
			{
				snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", boilerTemperature[i]);
				snprintf(topic, TOPIC_BUFFER_SIZE, "%s/boiler/temp/%u", MQTT_CLIENT_NAME, i);
				mqttClient.publish(topic, msg);
				_PMP(topic); _PP(" = ");  _PL(msg);
			}

		for (uint8_t i = 0; i < ACTUAL_HEAT_TEMPERATURE_SENSORS; i++)
			if (heatTemperature[i] > 0)
			{
				snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", heatTemperature[i]);
				snprintf(topic, TOPIC_BUFFER_SIZE, "%s/heat/temp/%u", MQTT_CLIENT_NAME, i);
				mqttClient.publish(topic, msg);
				_PMP(topic); _PP(" = ");  _PL(msg);
			}
	}
}
//Task taskSendTemperature(TASK_IMMEDIATE, TASK_ONCE, &OnSendTemperature, &ts);
Task taskSendTemperature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &OnSendTemperature, &ts);

void OnGetTempereature()
{
	_PML("Geting temperatures... ");

	for (int i = 0; i < ACTUAL_BOILER_TEMPERATURE_SENSORS; i++)
	{
		boilerTemperature[i] = temperatureSensors.getTempC(boilerThermometer[i]);
	}

	for (int i = 0; i < ACTUAL_HEAT_TEMPERATURE_SENSORS; i++)
	{
		heatTemperature[i] = temperatureSensors.getTempC(heatThermometer[i]);
	}

	//taskShowTempereature.restartDelayed();
	//taskSendTemperature.restartDelayed();
}
Task taskGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &OnGetTempereature, &ts);

void OnPrepareTemperature()
{
	_PL();  _PML("Requesting temperatures... ");
	temperatureSensors.requestTemperatures();
	taskGetTempereature.restartDelayed(750 * TASK_MILLISECOND);

}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnPrepareTemperature, &ts);



void setup()
{
	delay(1000);
	Serial.begin(COM_SPEED);
	delay(100);
	_PL(""); _PML("Programm started...");

	tConnectWiFi.enable();

	mqttClient.setServer(MQTT_SERVER, 1883);
	//mqttClient.setCallback(mqtt_callback);

	tConnectMQTT.enable();

#ifdef _TEST_
	tSendTest.enableDelayed();
#endif // _TEST_

	for (uint8_t i = 0; i < ACTUAL_BOILER_TEMPERATURE_SENSORS; i++)
	{
		temperatureSensors.setResolution(boilerThermometer[i], TEMPERATURE_PRECISION);
	}

	for (uint8_t i = 0; i < ACTUAL_HEAT_TEMPERATURE_SENSORS; i++)
	{
		temperatureSensors.setResolution(heatThermometer[i], TEMPERATURE_PRECISION);
	}

	temperatureSensors.setWaitForConversion(false);

	taskPrepareTempereature.enableDelayed();
	taskSendTemperature.enableDelayed();

}

void loop()
{
	ts.execute();
}
