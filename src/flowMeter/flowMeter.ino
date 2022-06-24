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
	#define ONE_WIRE_BUS D5
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
	#define COM_SPEED 115200
	#define ONE_WIRE_BUS 23
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
#else
	#define PRIMARY_SSID "PAGRABS"
	#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _TEST_

#define MQTT_SERVER "10.20.30.60"
#define CONNECTION_TIMEOUT 5

#define TEMPERATURE_PRECISION 12
#define TEMPERATURE_READ_PERIOD 10
#define BNT_DEBOUNCE  10 //20

#define FLOW_COUNTERS 2

Scheduler ts;
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


volatile int8_t btnOnISR = (-1);

#if defined(ARDUINO_ARCH_ESP8266)
	volatile uint8_t btnPins[FLOW_COUNTERS] = { D7, D6 };
#elif defined(ARDUINO_ARCH_ESP32)
	volatile uint8_t btnPins[FLOW_COUNTERS] = { 19, 18 };
#endif

volatile uint32_t btnPressedTime[FLOW_COUNTERS] = { 0, 0 }, btnPreviousPressedTime[FLOW_COUNTERS] = { 0, 0 }, btnDurationBetweenPresses[FLOW_COUNTERS] = { 0, 0 };
volatile uint16_t flowMeterValue[FLOW_COUNTERS] = { 0, 0 }, flowSpeed[FLOW_COUNTERS] = { 0, 0 };

void OnConnectWiFi();
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);

void OnConnectMQTT();
Task tConnctMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);

void OnButtonPressed();
Task tButtonPressed(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed, &ts);

void OnButtonReleased();
Task tButtonReleased(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased, &ts);

void FlowCorrection();
Task tFlowCorrection(30 * TASK_SECOND, TASK_FOREVER, &FlowCorrection, &ts);

void GetTempereature();
Task tGetTempereature(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &GetTempereature, &ts);

void OutputResult();
Task tOutputResult(TEMPERATURE_READ_PERIOD* TASK_SECOND, TASK_FOREVER, &OutputResult, &ts);

void isrAction(uint8_t pin)
{
	btnOnISR = pin;
	btnPressedTime[pin] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	//detachInterrupt(digitalPinToInterrupt(btnPins[1]));
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
	//detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	//detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	_PN("releaseButtonISR");
	tButtonReleased.restartDelayed(BNT_DEBOUNCE);
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
	_PP("IP address: "); _PL(WiFi.localIP());

	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

}

void OnConnectMQTT()
{
	if (!mqttClient.connected())
	{
		_PM("Attempting MQTT connection... ");
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

void OnButtonPressed()
{
	_PN("tButtonPressed:OnButtonPressed");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &releaseButtonISR, FALLING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[1]), &releaseButtonISR, FALLING);
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
	//attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, RISING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, RISING);
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

void OutputResult()
{
	if (mqttClient.connected())
	{
		_PL();
		for (int i = 0; i < 1; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish("flowmeter/0/flow", msg);
			_PM("flowmeter/0/flow = "); _PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			mqttClient.publish("flowmeter/0/value", msg);
			_PM("flowmeter/0/value = "); _PL(msg);
		}

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempInCommon);
		mqttClient.publish("flowmeter/0/temp/in", msg);
		_PM("flowmeter/0/temp/in = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempOutCommon);
		mqttClient.publish("flowmeter/0/temp/out", msg);
		_PM("flowmeter/0/temp/out = "); _PL(msg);

		snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", tempDiffCommon);
		mqttClient.publish("flowmeter/0/temp/diff", msg);
		_PM("flowmeter/0/temp/diff = "); _PL(msg);

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
