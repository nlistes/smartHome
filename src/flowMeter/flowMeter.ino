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
//#define _MQTT_TEST_
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

void OnWiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, HIGH);
	_S_PMP("Connected to: ");  _S_PL(PRIMARY_SSID);
}

void OnWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
	_S_PMP("IP address: "); _S_PL(WiFi.localIP());
}

void OnWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, LOW);
	//_S_PMP("Disconnected! Reason: "); _S_PL(info.wifi_sta_disconnected.reason);
}

#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "espTask_test"
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

void OnConnectMQTT()
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
Task tConnectMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);

void OnRunMQTT()
{
	mqttClient.loop();
}
Task tRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &OnRunMQTT, &ts);

#ifdef _TEST_
#define DATA_SEND_INTERVAL	60
uint16_t test_value = 0;

void OnGetTestValue()
{
	test_value++;
	_I_PMP("Counter: "); _I_PL(test_value);
}
Task tGetTestValue(DATA_SEND_INTERVAL* TASK_SECOND, TASK_FOREVER, &OnGetTestValue, &ts);

void OnSendTest()
{
	OnConnectMQTT();
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "%d", test_value);
		snprintf(topic, TOPIC_BUFFER_SIZE, "test/test2");
		mqttClient.publish(topic, msg);
		_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
	}
}
Task tSendTest(DATA_SEND_INTERVAL* TASK_SECOND, TASK_FOREVER, &OnSendTest, &ts);
#endif // _TEST_


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
	_PL();
	for (int i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
		_PM("Sensor"); _PP("["); _PP(i); _PP("] flow: "); _PP(flowSpeed[i]); _PP(" value: "); _PL(flowMeterValue[i]);
	}
}
Task tShowFlow(FLOW_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &ShowFlow, &ts);

void SendFlow()
{
	if (mqttClient.connected())
	{
		_PL();
		flowMeterValueSummary = 0;
		flowSpeedSummary = 0;
		for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/flow", MQTT_CLIENT_NAME, i);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish(topic, msg);
#ifdef _TEST_
			_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_


			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/value", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
#ifdef _TEST_
			_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_

			flowSpeedSummary = flowSpeedSummary + flowSpeed[i];
			flowMeterValueSummary = flowMeterValueSummary + flowMeterValue[i];
		}

		snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeedSummary);
		snprintf(topic, TOPIC_BUFFER_SIZE, "%s/summary/flow", MQTT_CLIENT_NAME);
		mqttClient.publish(topic, msg);
#ifdef _TEST_
		_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_

		snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValueSummary);
		snprintf(topic, TOPIC_BUFFER_SIZE, "%s/summary/value", MQTT_CLIENT_NAME);
		mqttClient.publish(topic, msg);
#ifdef _TEST_
		_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_
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
	_PL();
	_PN("OnButtonPressed_0");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_0, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_1()
{
	int8_t btnOnISR = 1;
	_PL();
	_PN("OnButtonPressed_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_1, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_2()
{
	int8_t btnOnISR = 2;
	_PL();
	_PN("OnButtonPressed_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_2, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_3()
{
	int8_t btnOnISR = 3;
	_PL();
	_PN("OnButtonPressed_3");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_3, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_4()
{
	int8_t btnOnISR = 4;
	_PL();
	_PN("OnButtonPressed_4");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_4, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonReleased_0()
{
	int8_t btnOnISR = 0;
	_PL();
	_PN("OnButtonReleased_0");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_0, RISING);
}

void OnButtonReleased_1()
{
	int8_t btnOnISR = 1;
	_PL();
	_PN("OnButtonReleased_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_1, RISING);
}

void OnButtonReleased_2()
{
	int8_t btnOnISR = 2;
	_PL();
	_PN("OnButtonReleased_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_2, RISING);
}

void OnButtonReleased_3()
{
	int8_t btnOnISR = 3;
	_PL();
	_PN("OnButtonReleased_3");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_3, RISING);
}

void OnButtonReleased_4()
{
	int8_t btnOnISR = 4;
	_PL();
	_PN("OnButtonReleased_4");
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

#define MAX_TEMPERATURE_SENSORS 5
#define ACTUAL_TEMPERATURE_SENSORS 5

DeviceAddress inThermometer[MAX_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x0C, 0xF5, 0x66, 0x04, 0x00, 0x00, 0x10 }, // inMain   0  =  2
	{ 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }, // inBoiler 1  = 11
	{ 0x28, 0x2E, 0xE1, 0x66, 0x04, 0x00, 0x00, 0xAB }, // inFloor  2  = 10
	{ 0x28, 0x42, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xC0 }, // in2.st   3  =  5
	{ 0x28, 0xBB, 0xF4, 0x66, 0x04, 0x00, 0x00, 0x5F }  // inMain2  4  = 15
};
DeviceAddress outThermometer[MAX_TEMPERATURE_SENSORS] =
{
	{ 0x28, 0x03, 0xCB, 0xE6, 0x03, 0x00, 0x00, 0xB1 }, // outMain   0 = 13
	{ 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }, // outBoiler 1 =  6
	{ 0x28, 0x02, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xB5 }, // outFloor  2 =  4
	{ 0x28, 0xA7, 0xE7, 0x66, 0x04, 0x00, 0x00, 0x4B }, // out2.st   3 = 16
	{ 0x28, 0xFA, 0xA7, 0x97, 0x04, 0x00, 0x00, 0x38 }  // inMain2   4 =  9
};

volatile float tempIn[MAX_TEMPERATURE_SENSORS], tempOut[MAX_TEMPERATURE_SENSORS], tempDiff[MAX_TEMPERATURE_SENSORS];

void GetTempereature()
{
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		tempIn[i] = sensors.getTempC(inThermometer[i]);
		tempOut[i] = sensors.getTempC(outThermometer[i]);
		tempDiff[i] = tempIn[i] - tempOut[i];
	}
}
Task tGetTempereature(TASK_IMMEDIATE, TASK_ONCE, &GetTempereature, &ts);

void OnPrepareTemperature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");
}
Task taskPrepareTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnPrepareTemperature, &ts);

void OnShowTemperature()
{
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		_PM("Sensor ");  _PP("[");  printOneWireAddress(inThermometer[i]); _PP("] temp IN: "); _PL(tempIn[i]);
		_PM("Sensor ");  _PP("[");  printOneWireAddress(outThermometer[i]); _PP("] temp OUT: "); _PL(tempOut[i]);
		_PM("Temperature difference: "); _PL(tempDiff[i]);
	}
}
Task taskShowTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnShowTemperature, &ts);

void OnSendTemperature()
{
	if (mqttClient.connected())
	{
		_PL();
		for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
			if ((tempIn[i] > 0) && (tempOut[i] > 0))
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempIn[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/in", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
#ifdef _TEST_
			_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_

			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOut[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/out", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
#ifdef _TEST_
			_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_
			snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiff[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%u/temp/diff", MQTT_CLIENT_NAME, i);
			mqttClient.publish(topic, msg);
#ifdef _TEST_
			_PM(topic); _PP(" = ");  _PL(msg);
#endif // _TEST_
		}
	}
}
Task taskSendTemperature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &OnSendTemperature, &ts);


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

	mqttClient.setServer(MQTT_SERVER, 1883);
	tConnectMQTT.enable();

	for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
		btnPressedTime[i] = 0;
		btnPreviousPressedTime[i] = 0;
		btnDurationBetweenPresses[i] = 0;
		flowMeterValue[i] = 0;
		flowSpeed[i] = 0;

		_PP("btnPins["); _PP(i); _PP("] = "); _PP(btnPins[i]); _PP("; digitalPinToInterrupt = "); _PL(digitalPinToInterrupt(btnPins[i]));
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

	taskPrepareTempereature.enableDelayed();
	taskShowTempereature.enableDelayed();
	taskSendTemperature.enableDelayed();
}


void loop()
{
	mqttClient.loop();
	ts.execute();
}



