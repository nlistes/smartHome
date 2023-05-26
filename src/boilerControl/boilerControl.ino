#include <OneWire.h>
#include <DallasTemperature.h>

#include <Bounce2.h>

// BEGIN TEMPLATE
// !!! Do not make changes! Update from espTask.ino

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <TaskScheduler.h>
#include <PubSubClient.h>

//#include <stdlib.h>
//#include <stdio.h>

// ==== Debug options ==================
#define _DEBUG_SYSTEM_
#define _DEBUG_INTERNAL_
//#define _DEBUG_EXTERNAL_

//===== Debug level SYSTEM ========================
#ifdef _DEBUG_SYSTEM_
#define SerialS Serial
#define _S_PP(a) SerialS.print(a)
#define _S_PL(a) SerialS.println(a)
#define _S_PH(a) SerialS.print(a, HEX)
#define _S_PMP(a) SerialS.print(millis()); SerialS.print(": "); SerialS.print(a)
#define _S_PML(a) SerialS.print(millis()); SerialS.print(": "); SerialS.println(a)
#else
#define _S_PP(a)
#define _S_PL(a)
#define _S_PH(a)
#define _S_PMP(a)
#define _S_PML(a)
#endif

//===== Debug level INTERNAL ========================
#ifdef _DEBUG_INTERNAL_
#define SerialI Serial
#define _I_PP(a) SerialI.print(a)
#define _I_PL(a) SerialI.println(a)
#define _I_PH(a) SerialI.print(a, HEX)
#define _I_PMP(a) SerialI.print(millis()); SerialI.print(": "); SerialI.print(a)
#define _I_PML(a) SerialI.print(millis()); SerialI.print(": "); SerialI.println(a)
#else
#define _I_PP(a)
#define _I_PL(a)
#define _I_PH(a)
#define _I_PMP(a)
#define _I_PML(a)
#endif

//===== Debug level EXTERNAL ========================
#ifdef _DEBUG_EXTERNAL_
#define SerialE Serial
#define _E_PP(a) SerialE.print(a)
#define _E_PL(a) SerialE.println(a)
#define _E_PH(a) SerialE.print(a, HEX)
#define _E_PMP(a) SerialE.print(millis()); SerialE.print(": "); SerialE.print(a)
#define _E_PML(a) SerialE.print(millis()); SerialE.print(": "); SerialE.println(a)
#else
#define _E_PP(a)
#define _E_PL(a)
#define _E_PH(a)
#define _E_PMP(a)
#define _E_PML(a)
#endif

// ==== Test options ==================
#define _TEST_
#define _MQTT_TEST_
#define _WIFI_TEST_

#if defined(ARDUINO_ARCH_ESP8266)
#define COM_SPEED 74880
WiFiClient ethClient;
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define COM_SPEED 115200
WiFiClient ethClient;
#endif

#ifdef _WIFI_TEST_
#define PRIMARY_SSID "PAGRABS2"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#else
#define PRIMARY_SSID "PAGRABS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _WIFI_TEST_

#define CONNECTION_TIMEOUT 10

Scheduler ts;

void onWiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, HIGH);
	_S_PMP("Connected to: ");  _S_PL(PRIMARY_SSID);
}

void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
	_S_PMP("IP address: "); _S_PL(WiFi.localIP());
	onConnectMQTT();
}

void onWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, LOW);
	//_S_PMP("Disconnected! Reason: "); _S_PL(info.wifi_sta_disconnected.reason);
}

#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "espTask_test"
#else
#define MQTT_SERVER "10.20.30.71"
#define MQTT_CLIENT_NAME "espTask-"
#endif // _MQTT_TEST_

PubSubClient mqttClient(ethClient);

String mqttClientId = MQTT_CLIENT_NAME;

#define MSG_BUFFER_SIZE	10
char msg[MSG_BUFFER_SIZE];

#define TOPIC_BUFFER_SIZE	40
char topic[TOPIC_BUFFER_SIZE];

void onConnectMQTT()
{
	if (!mqttClient.connected())
	{
		_S_PL();  _S_PMP("Connecting ");  _S_PP(mqttClientId.c_str()); _S_PP(" to MQTT["); _S_PP(MQTT_SERVER); _S_PP("] ");
		if (mqttClient.connect(mqttClientId.c_str()))
		{
			_S_PL("MQTT CONNECTED!");
			mqttClient.subscribe("boiler/+");
		}
		else
		{
			_S_PP("FAILED!!! rc="); _S_PL(mqttClient.state());
		}
	}
}
Task taskConnectMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &onConnectMQTT, &ts);

void onRunMQTT()
{
	mqttClient.loop();
}
Task taskRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &onRunMQTT, &ts);
// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE

#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 23
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&oneWire);

#define TEMPERATURE_PRECISION 10
#define TEMPERATURE_READ_PERIOD 15


#define MAX_TEMPERATURE_SENSORS 5
#define ACTUAL_TEMPERATURE_SENSORS 0

struct Thermometer
{
	DeviceAddress id;
	String name;
	float value;
};

Thermometer boilerThermometers[MAX_TEMPERATURE_SENSORS] =
{
	{{ 0x28, 0x50, 0xCE, 0x66, 0x04, 0x00, 0x00, 0xB7 }, "boiler_top", 0}, // 0
	{{ 0x28, 0xFC, 0xCE, 0x66, 0x04, 0x00, 0x00, 0x96 }, "boiler_bottom", 0}, // 3
	{{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, "heat_direct", 0},
	{{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, "heat_return", 0},
	{{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, "water", 0}
};


void onSendTemperature()
{
	onConnectMQTT();
	if (mqttClient.connected())
	{
		for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
			if (boilerThermometers[i].value > 0)
			{
				snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", boilerThermometers[i].value);
				snprintf(topic, TOPIC_BUFFER_SIZE, "%s/boiler/temp/%u", MQTT_CLIENT_NAME, i);
				//	mqttClient.publish(topic, msg);
				//	_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
			}
	}
}
Task taskSendTemperature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_ONCE, &onSendTemperature, &ts);

void onShowTemperature()
{
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		_I_PMP("Temperature [");  _I_PP(boilerThermometers[i].name); _I_PP("] = "); _I_PL(boilerThermometers[i].value);
	}
}
Task taskShowTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_ONCE, &onShowTemperature, &ts);

void onGetTempereature()
{
	_I_PML("Geting temperatures... ");

	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		boilerThermometers[i].value = temperatureSensors.getTempC(boilerThermometers[i].id);
	}
}
Task taskGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &onGetTempereature, &ts);

void onPrepareTemperature()
{
	_I_PL();  _I_PML("Requesting temperatures... ");
	temperatureSensors.requestTemperatures();
	taskGetTempereature.restartDelayed(750 * TASK_MILLISECOND);
}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onPrepareTemperature, &ts);

#define VALVE_PIN 17
#define SWITCH_PIN 16

Bounce boilerForceSwitch = Bounce(SWITCH_PIN, 5);

void onBoilerForceSwitch()
{
	boilerForceSwitch.update();
}
Task taskBoilerForceSwitch(TASK_IMMEDIATE, TASK_FOREVER, &onBoilerForceSwitch, &ts);

bool heatRequired, heatRequiredPrev;
bool heatRequiredChanged, heatRequiredChangedBySwitch, heatRequiredChangedByMQTT;

void onCheckHeatRequiredStatus()
{
	heatRequiredChanged = heatRequiredChangedBySwitch || heatRequiredChangedByMQTT;
	heatRequiredChangedByMQTT = false;
}
Task taskCheckHeatRequiredStatus(TASK_IMMEDIATE, TASK_FOREVER, &onCheckHeatRequiredStatus, &ts);


void onReadSwitch()
{
	heatRequiredChangedBySwitch = boilerForceSwitch.rose();
	heatRequired = heatRequiredChangedBySwitch ? !heatRequired : heatRequired;
}
Task taskReadSwitch(TASK_IMMEDIATE, TASK_FOREVER, &onReadSwitch, &ts);

void onRunValve()
{
	digitalWrite(VALVE_PIN, heatRequired);
}
Task taskRunValve(TASK_IMMEDIATE, TASK_FOREVER, &onRunValve, &ts);

void onShowValveStatus()
{
	if (heatRequiredChanged)
	{
		_I_PMP("heatRequired: "); _I_PL(heatRequired);
	}
}
Task taskShowValveStatus(TASK_IMMEDIATE, TASK_FOREVER, &onShowValveStatus, &ts);

void sendValveStatus()
{
	snprintf(msg, MSG_BUFFER_SIZE, "%u", heatRequired);
	snprintf(topic, TOPIC_BUFFER_SIZE, "boiler/status/heatRequired");
	mqttClient.publish(topic, msg);
	_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
}

void onSendValveStatus()
{
	if (heatRequiredChanged)
	{
		sendValveStatus();
	}
}
Task taskSendValveStatus(TASK_IMMEDIATE, TASK_FOREVER, &onSendValveStatus, &ts);

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
	_I_PMP("Message arrived ["); _I_PP(topic); _I_PP("] ");
	for (int i = 0; i < length; i++)
	{
		_I_PP((char)payload[i]);
	}
	_I_PL();

	if (strcmp(topic, "boiler/heatRequired") == 0)
	{
		heatRequiredChangedByMQTT = heatRequired != (char)payload[0];
		heatRequired = (char)payload[0] == '1';
	}

	if (strcmp(topic, "boiler/askStatus") == 0)
	{
		sendValveStatus();
	}
}


void setup()
{
	// BEGIN TEMPLATE
	// !!! Do not make changes! Update from espTask.ino
	Serial.begin(COM_SPEED);
	delay(100);
	randomSeed(analogRead(0));
	pinMode(LED_BUILTIN, OUTPUT);

	_S_PL(""); _S_PML("Programm started!");

	WiFi.onEvent(onWiFiConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent(onWiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent(onWiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

	WiFi.mode(WIFI_STA);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	mqttClientId = mqttClientId + String(random(0xffff), HEX);;
	mqttClient.setServer(MQTT_SERVER, 1883);
	mqttClient.setCallback(mqtt_callback);
	//tConnectMQTT.enable();
	taskRunMQTT.enable();
	// !!! Do not make changes! Update from espTask.ino
	// END TEMPLATE

	for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		temperatureSensors.setResolution(boilerThermometers[i].id, TEMPERATURE_PRECISION);
	}


	temperatureSensors.setWaitForConversion(false);
	//taskPrepareTempereature.enableDelayed();
	pinMode(SWITCH_PIN, INPUT);
	pinMode(VALVE_PIN, OUTPUT);
	taskBoilerForceSwitch.enableDelayed();
	taskReadSwitch.enableDelayed();
	taskRunValve.enableDelayed();
	taskShowValveStatus.enableDelayed();
	taskSendValveStatus.enableDelayed();
	taskCheckHeatRequiredStatus.enableDelayed();
}

void loop()
{
	ts.execute();
}
