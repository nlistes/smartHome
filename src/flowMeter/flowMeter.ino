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

#include <OneWire.h>
#include <DallasTemperature.h>

//#include <stdlib.h>
//#include <stdio.h>

// ==== Debug options ==================
#define _DEBUG_SYSTEM_
//#define _DEBUG_INTERNAL_
#define _DEBUG_EXTERNAL_

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
//#define _WIFI_TEST_

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
#define MQTT_CLIENT_NAME "flowMeter_test"
#else
#define MQTT_SERVER "10.20.30.81"
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
			//mqttClient.subscribe("inTopic");
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


#define MAX_FLOW_COUNTERS 5
#define ACTUAL_FLOW_COUNTERS 5
#define FLOW_READ_PERIOD 10
#define BNT_DEBOUNCE  10 //20

#if defined(ARDUINO_ARCH_ESP8266)
volatile uint8_t btnPins[MAX_FLOW_COUNTERS] = { D1, D2, D5, D6, D7 }; // Boileris, Bypass, Floor, 1_floor, 2_floor
#elif defined(ARDUINO_ARCH_ESP32)
volatile uint8_t btnPins[MAX_FLOW_COUNTERS] = { 22, 21, 18, 19, 23 };
#endif

volatile uint32_t btnPressedTime[MAX_FLOW_COUNTERS], btnPreviousPressedTime[MAX_FLOW_COUNTERS], btnDurationBetweenPresses[MAX_FLOW_COUNTERS];
volatile uint16_t flowMeterValue[MAX_FLOW_COUNTERS], flowSpeed[MAX_FLOW_COUNTERS];
volatile uint16_t flowMeterValueSummary, flowSpeedSummary;

void OnButtonPressed_0();
Task tButtonPressed_0(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_0, &ts);
void OnButtonReleased_0();
Task tButtonReleased_0(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_0, &ts);

void OnButtonPressed_1();
Task tButtonPressed_1(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_1, &ts);
void OnButtonPressed_2();
Task tButtonPressed_2(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_2, &ts);
void OnButtonPressed_3();
Task tButtonPressed_3(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_3, &ts);
void OnButtonPressed_4();
Task tButtonPressed_4(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_4, &ts);

void OnButtonReleased_1();
Task tButtonReleased_1(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_1, &ts);
void OnButtonReleased_2();
Task tButtonReleased_2(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_2, &ts);
void OnButtonReleased_3();
Task tButtonReleased_3(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_3, &ts);
void OnButtonReleased_4();
Task tButtonReleased_4(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_4, &ts);


void FlowCorrection()
{
	for (int i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
		if ((millis() - btnPreviousPressedTime[i]) > btnDurationBetweenPresses[i] & btnPreviousPressedTime[i] != 0)
		{
			flowSpeed[i] = (60 * 60 * 1000) / (millis() - btnPreviousPressedTime[i]);
		}
}
Task tFlowCorrection(1 * TASK_SECOND, TASK_FOREVER, &FlowCorrection, &ts);

void ShowFlow()
{
	_I_PL();
	for (int i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
		_I_PMP("flowMeter"); _I_PP("["); _I_PP(i); _I_PP("] flow: "); _I_PP(flowSpeed[i]); _I_PP(" value: "); _I_PL(flowMeterValue[i]);
	}
}
Task tShowFlow(FLOW_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &ShowFlow, &ts);

void SendFlow()
{
	if (mqttClient.connected())
	{
		_E_PL();
		flowMeterValueSummary = 0;
		flowSpeedSummary = 0;
		for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/flow", MQTT_CLIENT_NAME, i);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);


			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/value", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

			flowSpeedSummary = flowSpeedSummary + flowSpeed[i];
			flowMeterValueSummary = flowMeterValueSummary + flowMeterValue[i];
		}

		snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeedSummary);
		snprintf(topic, TOPIC_BUFFER_SIZE, "%s/summary/flow", MQTT_CLIENT_NAME);
		mqttClient.publish(topic, msg);
		_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValueSummary);
		snprintf(topic, TOPIC_BUFFER_SIZE, "%s/summary/value", MQTT_CLIENT_NAME);
		mqttClient.publish(topic, msg);
		_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
	}
}
Task tSendFlow(FLOW_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &SendFlow, &ts);

void IRAM_ATTR pressButtonISR_0()
{
	int8_t btnOnISR = 0;
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonPressed_0.restartDelayed(BNT_DEBOUNCE);

}

void IRAM_ATTR releaseButtonISR_0()
{
	int8_t btnOnISR = 0;
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonReleased_0.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR pressButtonISR_1()
{
	int8_t btnOnISR = 1;
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonPressed_1.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR pressButtonISR_2()
{
	int8_t btnOnISR = 2;
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonPressed_2.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR pressButtonISR_3()
{
	int8_t btnOnISR = 3;
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonPressed_3.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR pressButtonISR_4()
{
	int8_t btnOnISR = 4;
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonPressed_4.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_1()
{
	int8_t btnOnISR = 1;
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonReleased_1.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_2()
{
	int8_t btnOnISR = 2;
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonReleased_2.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_3()
{
	int8_t btnOnISR = 3;
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonReleased_3.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_4()
{
	int8_t btnOnISR = 4;
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
	tButtonReleased_4.restartDelayed(BNT_DEBOUNCE);
}

void OnButtonPressed_0()
{
	int8_t btnOnISR = 0;
	_I_PL();
	_I_PML("OnButtonPressed_0");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_0, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_1()
{
	int8_t btnOnISR = 1;
	_I_PL();
	_I_PML("OnButtonPressed_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_1, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_2()
{
	int8_t btnOnISR = 2;
	_I_PL();
	_I_PML("OnButtonPressed_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_2, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_3()
{
	int8_t btnOnISR = 3;
	_I_PL();
	_I_PML("OnButtonPressed_3");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_3, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_4()
{
	int8_t btnOnISR = 4;
	_I_PL();
	_I_PML("OnButtonPressed_4");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_4, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
}

void OnButtonReleased_0()
{
	int8_t btnOnISR = 0;
	_I_PL();
	_I_PML("OnButtonReleased_0");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_0, RISING);
}

void OnButtonReleased_1()
{
	int8_t btnOnISR = 1;
	_I_PL();
	_I_PML("OnButtonReleased_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_1, RISING);
}

void OnButtonReleased_2()
{
	int8_t btnOnISR = 2;
	_I_PL();
	_I_PML("OnButtonReleased_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_2, RISING);
}

void OnButtonReleased_3()
{
	int8_t btnOnISR = 3;
	_I_PL();
	_I_PML("OnButtonReleased_3");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_3, RISING);
}

void OnButtonReleased_4()
{
	int8_t btnOnISR = 4;
	_I_PL();
	_I_PML("OnButtonReleased_4");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_4, RISING);
}


#if defined(ARDUINO_ARCH_ESP8266)
#define ONE_WIRE_BUS D3
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define ONE_WIRE_BUS 17
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


#define MAX_TEMPERATURE_SENSORS 5
#define ACTUAL_TEMPERATURE_SENSORS 5

DeviceAddress inThermometer[MAX_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x0C, 0xF5, 0x66, 0x04, 0x00, 0x00, 0x10 }, // inMain   0  =  2
	{ 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }, // inBoiler 1  = 11
	{ 0x28, 0x2E, 0xE1, 0x66, 0x04, 0x00, 0x00, 0xAB }, // inFloor  2  = 10
	{ 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }, // in2.st   3  =  1
	{ 0x28, 0xBB, 0xF4, 0x66, 0x04, 0x00, 0x00, 0x5F }  // inMain2  4  = 15
};
DeviceAddress outThermometer[MAX_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x03, 0xCB, 0xE6, 0x03, 0x00, 0x00, 0xB1 }, // outMain   0 = 13
	{ 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }, // outBoiler 1 =  6
	{ 0x28, 0x02, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xB5 }, // outFloor  2 =  4
	{ 0x28, 0xA7, 0xE7, 0x66, 0x04, 0x00, 0x00, 0x4B }, // out2.st   3 = 16
	{ 0x28, 0xFA, 0xA7, 0x97, 0x04, 0x00, 0x00, 0x38 }  // outMain2  4 =  9
};

volatile float tempIn[MAX_TEMPERATURE_SENSORS], tempOut[MAX_TEMPERATURE_SENSORS], tempDiff[MAX_TEMPERATURE_SENSORS];

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 15


void onGetTempereature()
{
	_I_PML("onGetTempereature... ");
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		tempIn[i] = sensors.getTempC(inThermometer[i]);
		tempOut[i] = sensors.getTempC(outThermometer[i]);
		tempDiff[i] = tempIn[i] - tempOut[i];
	}
	_I_PML("DONE");
}
Task taskGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &onGetTempereature, &ts);

void onPrepareTemperature()
{
	_I_PL();  _I_PML("onPrepareTemperature... ");
	sensors.requestTemperatures();
	taskGetTempereature.restartDelayed(1 * TASK_SECOND);
	_I_PML("DONE");
}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &onPrepareTemperature, &ts);

void OnShowTemperature()
{
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		_I_PMP("Sensor ");  _I_PP("[");  printOneWireAddress(inThermometer[i]); _I_PP("] temp IN: "); _I_PL(tempIn[i]);
		_I_PMP("Sensor ");  _I_PP("[");  printOneWireAddress(outThermometer[i]); _I_PP("] temp OUT: "); _I_PL(tempOut[i]);
		_I_PMP("Temperature difference: "); _I_PL(tempDiff[i]);
	}
}
Task taskShowTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnShowTemperature, &ts);

void OnSendTemperature()
{
	if (mqttClient.connected())
	{
		_E_PL();
		for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
			if ((tempIn[i] > 0) && (tempOut[i] > 0))
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempIn[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/in", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOut[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/out", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiff[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/diff", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
		}
	}
}
Task taskSendTemperature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnSendTemperature, &ts);


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
	//mqttClient.setCallback(mqtt_callback);
	//tConnectMQTT.enable();
	taskRunMQTT.enable();
	// !!! Do not make changes! Update from espTask.ino
	// END TEMPLATE

	for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
		btnPressedTime[i] = 0;
		btnPreviousPressedTime[i] = 0;
		btnDurationBetweenPresses[i] = 0;
		flowMeterValue[i] = 0;
		flowSpeed[i] = 0;

		_S_PP("btnPins["); _S_PP(i); _S_PP("] = "); _S_PP(btnPins[i]); _S_PP("; digitalPinToInterrupt = "); _S_PL(digitalPinToInterrupt(btnPins[i]));
		pinMode(btnPins[i], INPUT);
	}

	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[3]), &pressButtonISR_3, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[4]), &pressButtonISR_4, FALLING);

	tFlowCorrection.enableDelayed();
	tShowFlow.enableDelayed();
	tSendFlow.enableDelayed();

	for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		sensors.setResolution(inThermometer[i], TEMPERATURE_PRECISION);
		sensors.setResolution(outThermometer[i], TEMPERATURE_PRECISION);
	}

	sensors.setWaitForConversion(false);

	taskPrepareTempereature.enableDelayed();
	taskShowTempereature.enableDelayed();
	taskSendTemperature.enableDelayed();
}


void loop()
{
	ts.execute();
}



