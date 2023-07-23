#include <OneWire.h>
#include <DallasTemperature.h>

#include <Bounce2.h>

#define HOSTNAME "ESP32-boilerControl"
#define DEVICE_TYPE "Boiler"
#define DEVICE_NAME "Pagrabs"
#define MQTT_IN_TOPIC "Boiler/#"

// BEGIN TEMPLATE
// !!! Do not make changes! Update from espTask.ino

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif

#include <TaskScheduler.h>
#include <PubSubClient.h>

//#include <stdlib.h>
//#include <stdio.h>

// ==== Debug options ==================
#define _DEBUG_
#define _DEBUG_SYSTEM_
#define _DEBUG_INTERNAL_
#define _DEBUG_EXTERNAL_

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

#ifndef HOSTNAME
#define HOSTNAME "ESP32-TASK"
#endif // !HOSTNAME

#ifndef DEVICE_TYPE
#define DEVICE_TYPE "Unknown"
#endif // !DEVICE_TYPE

#ifndef DEVICE_NAME
#define DEVICE_NAME "Unknown"
#endif // !DEVICE_TYPE


#if defined(ARDUINO_ARCH_ESP8266)
#define COM_SPEED 74880
WiFiClient ethClient;
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define COM_SPEED 115200
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
#define MQTT_SERVER "10.20.30.70"
#define MQTT_CLIENT_NAME "espTest-"
#else
#define MQTT_SERVER "10.20.30.80"
#define MQTT_CLIENT_NAME "espTask-"
#endif // _MQTT_TEST_

#ifndef MQTT_IN_TOPIC
#define MQTT_IN_TOPIC "$SYS/broker/version"
#endif // !MQTT_IN_TOPIC


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
			mqttClient.subscribe(MQTT_IN_TOPIC);
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

void onHandleOTA()
{
	ArduinoOTA.handle();
}
Task taskHandleOTA(TASK_IMMEDIATE, TASK_FOREVER, &onHandleOTA, &ts);

// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE


// ### TEMPERATURE START DEFINITIONS ###
#define TEMPERATURE_PRECISION 10
#define TEMPERATURE_READ_PERIOD 15

#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 18
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&oneWire);

struct Thermometer
{
	DeviceAddress id;
	String name;
	float value;
};

#define MAX_TEMPERATURE_SENSORS 5
#define ACTUAL_TEMPERATURE_SENSORS 1

Thermometer boilerThermometers[MAX_TEMPERATURE_SENSORS] =
{
	{{ 0x28, 0xDA, 0x4A, 0x9C, 0x04, 0x00, 0x00, 0x2C }, "water", 0}, // 8
	{{ 0x28, 0x13, 0x44, 0x67, 0x04, 0x00, 0x00, 0x62 }, "boiler_top", 0}, // 14
	{{ 0x28, 0x95, 0xC3, 0xBD, 0x04, 0x00, 0x00, 0xBA }, "boiler_case", 0}, // 12
	{{ 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }, "heat_direct", 0}, // 11
	{{ 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }, "heat_return", 0}  // 6
};


void onPrepareTemperature();
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onPrepareTemperature, &ts);

void onGetTempereature();
Task taskGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &onGetTempereature, &ts);

void onShowTemperature();
Task taskShowTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onShowTemperature, &ts);

void onSendTemperature();
Task taskSendTemperature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onSendTemperature, &ts);
// ### TEMPERATURE END DEFINITIONS###


// ### FLOW START ###
#define FLOW_COUNTER_READ_PERIOD 5
#define FLOW_COUNTER_DEBOUNCE_TIME  10 //20

#define MAX_FLOW_COUNTERS 2
#define ACTUAL_FLOW_COUNTERS 2

#define BOILER_OFF_FLOW_THRESHOLD 70

//#define USE_FLOW_STRUCT

struct flowMeter
{
	uint8_t pin;
	String name;
	uint16_t value;
	uint16_t flow;
	uint32_t previousPressTime;
	uint32_t durationBetweenPress;
};

volatile uint32_t pressTime[MAX_FLOW_COUNTERS];

flowMeter boilerFlowCounters[MAX_FLOW_COUNTERS] =
{
	{ 19, "heat", 0, 0, 0, 0 },
	{ 23, "water", 0, 0, 0, 0 }
};

#if defined(ARDUINO_ARCH_ESP8266)
volatile uint8_t btnPins[MAX_FLOW_COUNTERS] = { D1, D2, D5, D6, D7 }; // Boileris, Bypass, Floor, 1_floor, 2_floor
#elif defined(ARDUINO_ARCH_ESP32)
uint8_t btnPins[MAX_FLOW_COUNTERS] = { 19, 23 };
#endif

volatile uint32_t btnPressedTime[MAX_FLOW_COUNTERS];
uint32_t btnPreviousPressedTime[MAX_FLOW_COUNTERS], btnDurationBetweenPresses[MAX_FLOW_COUNTERS];
uint16_t flowMeterValue[MAX_FLOW_COUNTERS], flowSpeed[MAX_FLOW_COUNTERS];
String flowCounterName[MAX_FLOW_COUNTERS] = {"heat", "water"};

void IRAM_ATTR pressButtonISR_0();
void IRAM_ATTR pressButtonISR_1();

void IRAM_ATTR releaseButtonISR_0();
void IRAM_ATTR releaseButtonISR_1();

void onButtonPressed_0();
Task taskButtonPressed_0(TASK_IMMEDIATE, TASK_ONCE, &onButtonPressed_0, &ts);
void onButtonPressed_1();
Task taskButtonPressed_1(TASK_IMMEDIATE, TASK_ONCE, &onButtonPressed_1, &ts);

void onButtonReleased_0();
Task taskButtonReleased_0(TASK_IMMEDIATE, TASK_ONCE, onButtonReleased_0, &ts);
void onButtonReleased_1();
Task taskButtonReleased_1(TASK_IMMEDIATE, TASK_ONCE, &onButtonReleased_1, &ts);

void onFlowCorrection();
Task taskFlowCorrection(1 * TASK_SECOND, TASK_FOREVER, &onFlowCorrection, &ts);

void onShowFlow();
Task taskShowFlow(FLOW_COUNTER_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onShowFlow, &ts);

void onSendFlow();
Task taskSendFlow(FLOW_COUNTER_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onSendFlow, &ts);
// ### FLOW END ###


// ### VALVE START ###
#define VALVE_STATUS_READ_PERIOD 120

#define VALVE_PIN 17
#define SWITCH_PIN 16
#define LOCAL_STATUS_PIN 21
#define REMOTE_STATUS_PIN 22

bool heatRequired, heatRequiredPrev;
bool heatRequiredChanged, heatRequiredChangedBySwitch, heatRequiredChangedByMQTT, heatRequiredChangedByFlow;

Bounce boilerForceSwitch = Bounce(SWITCH_PIN, 5);

void onBoilerForceSwitch();
Task taskBoilerForceSwitch(TASK_IMMEDIATE, TASK_FOREVER, &onBoilerForceSwitch, &ts);

void onReadSwitch();
Task taskReadSwitch(TASK_IMMEDIATE, TASK_FOREVER, &onReadSwitch, &ts);

void onCheckHeatRequiredStatus();
Task taskCheckHeatRequiredStatus(TASK_IMMEDIATE, TASK_FOREVER, &onCheckHeatRequiredStatus, &ts);

void onRunValve();
Task taskRunValve(TASK_IMMEDIATE, TASK_FOREVER, &onRunValve, &ts);

void onShowValveStatus();
Task taskShowValveStatus(TASK_IMMEDIATE, TASK_FOREVER, &onShowValveStatus, &ts);

void onSendValveStatus();
Task taskSendValveStatus(VALVE_STATUS_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &onSendValveStatus, &ts);

void onCheckFlowThreshold();
Task taskCheckFlowThreshold(FLOW_COUNTER_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &onCheckFlowThreshold, &ts);
// ### VALVE END ###


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
	WiFi.setHostname(HOSTNAME);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	mqttClientId = mqttClientId + String(random(0xffff), HEX);;
	mqttClient.setServer(MQTT_SERVER, 1883);
	//mqttClient.setCallback(mqtt_callback);

	ArduinoOTA.setHostname(HOSTNAME);

	ArduinoOTA.onStart
	(
		[]()
		{
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";
			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("Start updating " + type);
		}
	);

	ArduinoOTA.onEnd
	(
		[]()
		{
			Serial.println("\nEnd");
		}
	);

	ArduinoOTA.onProgress
	(
		[](unsigned int progress, unsigned int total)
		{
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		}
	);

	ArduinoOTA.onError
	(
		[](ota_error_t error)
		{
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
		}
	);

	ArduinoOTA.begin();

	//tConnectMQTT.enable();
	taskRunMQTT.enable();
	taskHandleOTA.enable();
// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE

	mqttClient.setCallback(mqtt_callback);

	temperatureSensors.setWaitForConversion(false);
	for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		temperatureSensors.setResolution(boilerThermometers[i].id, TEMPERATURE_PRECISION);
	}

	taskPrepareTempereature.enableDelayed();
	taskShowTempereature.enableDelayed();
	taskSendTemperature.enableDelayed();

	_S_PL("");
	for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
#ifdef USE_FLOW_STRUCT
		_S_PP("btnPins["); _S_PP(i); _S_PP("] = "); _S_PP(boilerFlowCounters[i].pin); _S_PP("; digitalPinToInterrupt = "); _S_PL(digitalPinToInterrupt(boilerFlowCounters[i].pin));
		pinMode(boilerFlowCounters[i].pin, INPUT);
#else
		_S_PP("btnPins["); _S_PP(i); _S_PP("] = "); _S_PP(btnPins[i]); _S_PP("; digitalPinToInterrupt = "); _S_PL(digitalPinToInterrupt(btnPins[i]));
		pinMode(btnPins[i], INPUT);
#endif
	}

#ifdef USE_FLOW_STRUCT
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[0].pin), &pressButtonISR_0, FALLING);
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[1].pin), &pressButtonISR_1, FALLING);
#else
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, FALLING);
#endif

	taskFlowCorrection.enableDelayed();
	taskShowFlow.enableDelayed();
	taskSendFlow.enableDelayed();

	pinMode(SWITCH_PIN, INPUT);
	pinMode(VALVE_PIN, OUTPUT);
	pinMode(LOCAL_STATUS_PIN, OUTPUT);
	pinMode(REMOTE_STATUS_PIN, OUTPUT);
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


void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
	_I_PMP("Message arrived ["); _I_PP(topic); _I_PP("] ");
	for (int i = 0; i < length; i++)
	{
		_I_PP((char)payload[i]);
	}
	_I_PL();

	if (strcmp(topic, "Boiler/Pagrabs/request-heatRequired") == 0)
	{
		heatRequiredChangedByMQTT = heatRequired != (char)payload[0];
		heatRequired = (char)payload[0] == '1';
	}

	if (strcmp(topic, "Boiler/Pagrabs/request-valveStatus") == 0)
	{
		taskSendValveStatus.forceNextIteration();
	}
}
