// ==== Debug and Test options ==================
#define _DEBUG_
#define _TEST_

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

//#include <stdlib.h>
//#include <stdio.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WiFi.h>
	#define COM_SPEED 74880
	#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
	#define COM_SPEED 115200
	#define ONE_WIRE_BUS 23
#endif

#include <TaskScheduler.h>
Scheduler ts;

#ifdef _TEST_
	#define PRIMARY_SSID "OSIS"
	#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
	#define MQTT_SERVER "10.20.30.60"
	#define MQTT_CLIENT_NAME "flowMeter_test"
#else
	#define PRIMARY_SSID "PAGRABS"
	#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
	#define MQTT_SERVER "10.20.30.71"
	#define MQTT_CLIENT_NAME "flowMeter"
#endif // _TEST_

#define CONNECTION_TIMEOUT 5

#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 10
#define BNT_DEBOUNCE  10 //20

#define FLOW_COUNTERS 3

WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#ifdef _TEST_
	DeviceAddress inThermometerCommon = { 0x28, 0x88, 0xDC, 0x66, 0x04, 0x00, 0x00, 0x2D }; // 1
	DeviceAddress outThermometerCommon = { 0x28, 0x42, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xC0 }; // 5
	DeviceAddress inThermometerBoiler = { 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }; // 11
	DeviceAddress outThermometerBoiler = { 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }; // 6
	DeviceAddress inThermometerSmallCircle = { 0x28, 0x02, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xB5 }; // 4
	DeviceAddress outThermometerSmallCircle = { 0x28, 0x2E, 0xE1, 0x66, 0x04, 0x00, 0x00, 0xAB }; // 10
#else
	DeviceAddress inThermometerCommon = { 0x28, 0x0C, 0xF5, 0x66, 0x04, 0x00, 0x00, 0x10 }; // 2
	DeviceAddress outThermometerCommon = { 0x28, 0xFA, 0xA7, 0x97, 0x04, 0x00, 0x00, 0x38 }; // 9
	DeviceAddress inThermometerBoiler = { 0x28, 0x25, 0xDB, 0x66, 0x04, 0x00, 0x00, 0x6A }; // 11
	DeviceAddress outThermometerBoiler = { 0x28, 0xE2, 0xE1, 0x66, 0x04, 0x00, 0x00, 0x49 }; // 6
	DeviceAddress inThermometerSmallCircle = { 0x28, 0x02, 0xD6, 0x66, 0x04, 0x00, 0x00, 0xB5 }; // 4
	DeviceAddress outThermometerSmallCircle = { 0x28, 0x2E, 0xE1, 0x66, 0x04, 0x00, 0x00, 0xAB }; // 10

#endif // _TEST_


volatile float tempInCommon, tempOutCommon, tempDiffCommon;
volatile float tempInBoiler, tempOutBoiler, tempDiffBoiler;
volatile float tempInSmallCircle, tempOutSmallCircle, tempDiffSmallCircle;


#if defined(ARDUINO_ARCH_ESP8266)
	volatile uint8_t btnPins[FLOW_COUNTERS] = { D5, D6, D7 };
#elif defined(ARDUINO_ARCH_ESP32)
	volatile uint8_t btnPins[FLOW_COUNTERS] = { 19, 18, 20 };
#endif

volatile uint32_t btnPressedTime[FLOW_COUNTERS] = { 0, 0, 0 }, btnPreviousPressedTime[FLOW_COUNTERS] = { 0, 0, 0 }, btnDurationBetweenPresses[FLOW_COUNTERS] = { 0, 0, 0 };
volatile uint16_t flowMeterValue[FLOW_COUNTERS] = { 0, 0, 0 }, flowSpeed[FLOW_COUNTERS] = { 0, 0, 0 };

void OnConnectWiFi();
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);

void OnConnectMQTT();
Task tConnctMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);

void OnButtonPressed_0();
Task tButtonPressed_0(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_0, &ts);
void OnButtonPressed_1();
Task tButtonPressed_1(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_1, &ts);
void OnButtonPressed_2();
Task tButtonPressed_2(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed_2, &ts);


void OnButtonReleased_0();
Task tButtonReleased_0(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_0, &ts);
void OnButtonReleased_1();
Task tButtonReleased_1(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_1, &ts);
void OnButtonReleased_2();
Task tButtonReleased_2(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased_2, &ts);

void FlowCorrection();
Task tFlowCorrection(30 * TASK_SECOND, TASK_FOREVER, &FlowCorrection, &ts);

void GetTempereature();
Task tGetTempereature(TEMPERATURE_READ_PERIOD * TASK_SECOND, TASK_FOREVER, &GetTempereature, &ts);

void GetFlow();
Task tGetFlow(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &GetFlow, &ts);

void SendTemperature();
Task tSendTemperature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &SendTemperature, &ts);

void SendFlow();
Task tSendFlow(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &SendFlow, &ts);


void IRAM_ATTR pressButtonISR_0()
{
	btnPressedTime[0] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	tButtonPressed_0.restartDelayed(BNT_DEBOUNCE);

}

void IRAM_ATTR pressButtonISR_1()
{
	btnPressedTime[1] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	tButtonPressed_1.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR pressButtonISR_2()
{
	btnPressedTime[2] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	tButtonPressed_2.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_0()
{
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	tButtonReleased_0.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_1()
{
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	tButtonReleased_1.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR_2()
{
	detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	tButtonReleased_2.restartDelayed(BNT_DEBOUNCE);
}
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

void OnButtonPressed_0()
{
	int8_t btnOnISR = 0;
	_PN("OnButtonPressed_0");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_0, RISING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_1()
{
	int8_t btnOnISR = 1;
	_PN("OnButtonPressed_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_1, RISING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonPressed_2()
{
	int8_t btnOnISR = 2;
	_PN("OnButtonPressed_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_2, RISING);
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
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_0, FALLING);
}

void OnButtonReleased_1()
{
	int8_t btnOnISR = 1;
	_PL();
	_PN("OnButtonReleased_1");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_1, FALLING);
}

void OnButtonReleased_2()
{
	int8_t btnOnISR = 2;
	_PL();
	_PN("OnButtonReleased_2");
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_2, FALLING);
}

void FlowCorrection()
{
	if ((millis() - btnPreviousPressedTime[0]) > 90 * TASK_SECOND)
	{
		flowSpeed[0] = (60 * 60 * 1000) / (millis() - btnPreviousPressedTime[0]);
	}
}

void GetTempereature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");
	tempInCommon = sensors.getTempC(inThermometerCommon);
	tempOutCommon = sensors.getTempC(outThermometerCommon);
	tempDiffCommon = tempInCommon - tempOutCommon;
	_PM("Sensor ");  _PP("[");  printAddress(inThermometerCommon); _PP("] temp: "); _PL(tempInCommon);
	_PM("Sensor ");  _PP("[");  printAddress(outThermometerCommon); _PP("] temp: "); _PL(tempOutCommon);
	_PM("Temperature difference: "); _PL(tempDiffCommon);

	tempInBoiler = sensors.getTempC(inThermometerBoiler);
	tempOutBoiler = sensors.getTempC(outThermometerBoiler);
	tempDiffBoiler = tempInBoiler - tempOutBoiler;
	_PM("Sensor ");  _PP("[");  printAddress(inThermometerBoiler); _PP("] temp: "); _PL(tempInBoiler);
	_PM("Sensor ");  _PP("[");  printAddress(outThermometerBoiler); _PP("] temp: "); _PL(tempOutBoiler);
	_PM("Temperature difference: "); _PL(tempDiffBoiler);

	tempInSmallCircle = sensors.getTempC(inThermometerSmallCircle);
	tempOutSmallCircle = sensors.getTempC(outThermometerSmallCircle);
	tempDiffSmallCircle = tempInSmallCircle - tempOutSmallCircle;
	_PM("Sensor ");  _PP("[");  printAddress(inThermometerSmallCircle); _PP("] temp: "); _PL(tempInSmallCircle);
	_PM("Sensor ");  _PP("[");  printAddress(outThermometerSmallCircle); _PP("] temp: "); _PL(tempOutSmallCircle);
	_PM("Temperature difference: "); _PL(tempDiffSmallCircle);

}

void GetFlow()
{
	_PL();
	for (int i = 0; i < FLOW_COUNTERS; i++)
	{
		_PM("Sensor"); _PP("["); _PP(i); _PP("] flow: "); _PP(flowSpeed[i]); _PP(" value: "); _PL(flowMeterValue[i]);
	}
}

void SendTemperature()
{
	if (mqttClient.connected())
	{
		_PL();

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempInCommon);
		mqttClient.publish("flowmeter/common/temp/in", msg);
		_PM("flowmeter/common/temp/in = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOutCommon);
		mqttClient.publish("flowmeter/common/temp/out", msg);
		_PM("flowmeter/common/temp/out = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiffCommon);
		mqttClient.publish("flowmeter/common/temp/diff", msg);
		_PM("flowmeter/common/temp/diff = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempInBoiler);
		mqttClient.publish("flowmeter/1/temp/in", msg);
		_PM("flowmeter/1/temp/in = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOutBoiler);
		mqttClient.publish("flowmeter/1/temp/out", msg);
		_PM("flowmeter/1/temp/out = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiffBoiler);
		mqttClient.publish("flowmeter/1/temp/diff", msg);
		_PM("flowmeter/1/temp/diff = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempInSmallCircle);
		mqttClient.publish("flowmeter/2/temp/in", msg);
		_PM("flowmeter/2/temp/in = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOutSmallCircle);
		mqttClient.publish("flowmeter/2/temp/out", msg);
		_PM("flowmeter/2/temp/out = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiffSmallCircle);
		mqttClient.publish("flowmeter/2/temp/diff", msg);
		_PM("flowmeter/2/temp/diff = "); _PL(msg);

	}
}

void SendFlow()
{
	if (mqttClient.connected())
	{
		_PL();
		for (int i = 0; i < FLOW_COUNTERS; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish("flowmeter/0/flow", msg);
			_PM("flowmeter/0/flow = "); _PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			mqttClient.publish("flowmeter/0/value", msg);
			_PM("flowmeter/0/value = "); _PL(msg);
		}
	}
}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PL(); _PM("Starting application...");

	_PL();
	for (int i = 0; i < FLOW_COUNTERS; i++)
	{
		_PP("btnPins["); _PP(i); _PP("] = "); _PP(btnPins[i]); _PP("; digitalPinToInterrupt = "); _PL(digitalPinToInterrupt(btnPins[i]));
		pinMode(btnPins[i], INPUT);
	}

	sensors.setResolution(inThermometerCommon, TEMPERATURE_PRECISION);
	sensors.setResolution(outThermometerCommon, TEMPERATURE_PRECISION);
	sensors.setResolution(inThermometerBoiler, TEMPERATURE_PRECISION);
	sensors.setResolution(outThermometerBoiler, TEMPERATURE_PRECISION);
	sensors.setResolution(inThermometerSmallCircle, TEMPERATURE_PRECISION);
	sensors.setResolution(outThermometerSmallCircle, TEMPERATURE_PRECISION);

	mqttClient.setServer(MQTT_SERVER, 1883);
	tConnectWiFi.enable();
	tConnctMQTT.enable();
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, RISING);
	//tFlowCorrection.enable();
	//tGetTempereature.enable();
	tGetFlow.enable();
	//tSendTemperature.enable();
	//tSendFlow.enable();
}

void loop()
{
	mqttClient.loop();
	ts.execute();
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
