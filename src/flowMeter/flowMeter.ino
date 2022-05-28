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

//#include <stdlib.h>
//#include <stdio.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WiFi.h>
	#define COM_SPEED 74880
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
	#define COM_SPEED 115200
#endif

#include <PubSubClient.h>
#include <TaskScheduler.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

#define SSID "PAGRABS"
#define PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_SERVER "10.20.30.60"

WiFiClient ethClient;
PubSubClient mqttClient(ethClient);

#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define TEMPERATURE_PRECISION 12

DeviceAddress inThermometer = { 0x28, 0x0C, 0xF5, 0x66, 0x04, 0x00, 0x00, 0x10 }; // 2
DeviceAddress outThermometer = { 0x28, 0xFA, 0xA7, 0x97, 0x04, 0x00, 0x00, 0x38 }; // 9

volatile float tempInCommon, tempOutCommon, tempDiffCommon;

#define BNT_DEBOUNCE  10 //20

volatile int8_t btnOnISR = (-1);
volatile uint8_t btnPins[2] = { D7, D6};
volatile uint32_t btnPressedTime[2] = { 0, 0 }, btnPreviousPressedTime[3] = { 0, 0 }, btnDurationBetweenPresses[3] = { 0, 0 };
volatile uint16_t flowMeterValue[2] = { 0, 0 }, flowSpeed[2] = { 0, 0 };

Scheduler ts;
void OnConnectMQTT();
void OnButtonPressed();
void OnButtonReleased();
void FlowCorrection();
void GetTempereature();
void OutputResult();

Task tConnctMQTT(5 * TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);
Task tButtonPressed(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed, &ts);
Task tButtonReleased(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased, &ts);
Task tFlowCorrection(30 * TASK_SECOND, TASK_FOREVER, &FlowCorrection, &ts);
Task tGetTempereature(4 * TASK_SECOND, TASK_FOREVER, &GetTempereature, &ts);
Task tOutputResult(5 * TASK_SECOND, TASK_FOREVER, &OutputResult, &ts);

void isrAction(uint8_t pin)
{
	btnOnISR = pin;
	btnPressedTime[pin] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	//detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	_PN("isrAction");
	tButtonPressed.restartDelayed(BNT_DEBOUNCE);
}


void IRAM_ATTR pressButtonISR_0()
{
	isrAction(0);
}

void IRAM_ATTR pressButtonISR_1()
{
	isrAction(1);
}

void IRAM_ATTR pressButtonISR_2()
{
	isrAction(2);
}

void IRAM_ATTR releaseButtonISR()
{
	btnOnISR = (-1);
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	//detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	_PN("releaseButtonISR");
	tButtonReleased.restartDelayed(BNT_DEBOUNCE);
}

void OnButtonPressed()
{
	_PN("tButtonPressed:OnButtonPressed");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &releaseButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &releaseButtonISR, FALLING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[2]), &releaseButtonISR, FALLING);
	flowMeterValue[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	//tOutputResult.restartDelayed();
	_PM(flowMeterValue[btnOnISR]); _PP(": "); _PP(btnDurationBetweenPresses[btnOnISR]); _PP(": "); _PL(flowSpeed[btnOnISR]);
}

void OnButtonReleased()
{
	_PN("tButtonReleased:OnButtonReleased\n");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, RISING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, RISING);
}

void FlowCorrection()
{
	if ((millis() - btnPreviousPressedTime[0]) > 90 * TASK_SECOND)
	{
		flowSpeed[0] = (60 * 60 * 1000) / (millis() - btnPreviousPressedTime[0]);
	}
}

void OnConnectMQTT()
{
	if (!mqttClient.connected())
	{
		_PP("Attempting MQTT connection... ");
		if (mqttClient.connect("flowMeter"))
		{
			_PL(" connected!");
			mqttClient.subscribe("inTopic");
		}
		else
		{
			_PP("failed, rc="); _PL(mqttClient.state());
		}

	}

}

void GetTempereature()
{
	_PL();  _PN("Requesting temperatures... ");
	sensors.requestTemperatures();
	_PN("DONE");
	tempInCommon = sensors.getTempC(inThermometer);
	tempOutCommon = sensors.getTempC(outThermometer);
	tempDiffCommon = tempInCommon - tempOutCommon;
	_PM("Sensor ");  _PP("[");  printAddress(inThermometer); _PP("] temp: "); _PL(tempInCommon);
	_PM("Sensor ");  _PP("[");  printAddress(outThermometer); _PP("] temp: "); _PL(tempOutCommon);
	_PM("Temperature difference: "); _PL(tempDiffCommon);
}

void OutputResult()
{
	if (mqttClient.connected())
	{
		for (int i = 0; i < 1; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish("flowmeter/0/flow", msg);
			_PM("flowmeter/0/flow = "); _PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			mqttClient.publish("flowmeter/0/value", msg);
			_PM("flowmeter/0/value = "); _PL(msg); _PL();
		}

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempInCommon);
		//gcvt(inThemperature, 2, msg);
		mqttClient.publish("flowmeter/0/temp/in", msg);
		_PM("flowmeter/0/temp/in = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOutCommon);
		mqttClient.publish("flowmeter/0/temp/out", msg);
		_PM("flowmeter/0/temp/out = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiffCommon);
		mqttClient.publish("flowmeter/0/temp/diff", msg);
		_PM("flowmeter/0/temp/diff = "); _PL(msg);

	}
}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(100);
	_PM("Starting application...");
	sensors.setResolution(inThermometer, TEMPERATURE_PRECISION);
	sensors.setResolution(outThermometer, TEMPERATURE_PRECISION);

	_PL(); _PP("Connecting to "); _PP(SSID);
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID, PASS);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		_PP(".");
	}

	_PL("connected!");
	_PP("IP address: "); _PL(WiFi.localIP());

	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	mqttClient.setServer(MQTT_SERVER, 1883);
	tConnctMQTT.enable();
	tButtonReleased.enable();
	//tFlowCorrection.enable();
	tGetTempereature.enable();
	tOutputResult.enable();
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
