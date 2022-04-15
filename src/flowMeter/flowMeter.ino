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
#define _PH(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

#include <stdlib.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
#endif

#include <PubSubClient.h>
#include <TaskScheduler.h>

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

#define SSID "OSIS"
#define PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_SERVER "10.20.30.60"
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);

#define BNT_DEBOUNCE  10 //20

//#define BTN_SEL_PIN   D7
//#define BTN_PLUS_PIN  D6
//#define BTN_MINUS_PIN D5

//volatile long buttonD7PressedTime, buttonD7PreviousPressedTime, buttonD7TimeSlot;
volatile int8_t btnOnISR = (-1);
volatile uint8_t btnPins[3] = { D7, D6, D5 };
volatile uint32_t btnPressedTime[3] = { 0, 0, 0 }, btnPreviousPressedTime[3] = { 0, 0, 0 }, btnDurationBetweenPresses[3] = { 0, 0, 0 };

Scheduler ts;
void OnConnectMQTT();
void OnButtonPressed();
void OnButtonReleased();
void OutputResult();

Task tConnctMQTT(5 * TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);
Task tButtonPressed(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed, &ts);
Task tButtonReleased(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased, &ts);
Task tOutputResult(50 * TASK_MILLISECOND, TASK_FOREVER, &OutputResult, &ts);

//void IRAM_ATTR pressButtonISR()	// attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
//{
//	buttonD7PressedTime = millis();
//	detachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN));
//	_PM("pressButtonISR");
//	tButtonPressed.restartDelayed(BNT_DEBOUNCE);
//}

void isrAction(uint8_t pin)
{
	btnOnISR = pin;
	btnPressedTime[pin] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	_PN("isrAction");
	tButtonPressed.restartDelayed(BNT_DEBOUNCE);
}


void IRAM_ATTR pressButtonISR_0()	// attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
{
	isrAction(0);
}

void IRAM_ATTR pressButtonISR_1()	// attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
{
	isrAction(1);
}

void IRAM_ATTR pressButtonISR_2()	// attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
{
	isrAction(2);
}

void IRAM_ATTR releaseButtonISR()
{
	//detachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN));
	btnOnISR = (-1);
	detachInterrupt(digitalPinToInterrupt(btnPins[0]));
	detachInterrupt(digitalPinToInterrupt(btnPins[1]));
	detachInterrupt(digitalPinToInterrupt(btnPins[2]));
	_PN("releaseButtonISR");
	tButtonReleased.restartDelayed(BNT_DEBOUNCE);
}

void OnButtonPressed()
{
	_PN("tButtonPressed:OnButtonPressed");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &releaseButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &releaseButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(btnPins[2]), &releaseButtonISR, FALLING);
	//buttonD7TimeSlot = buttonD7PressedTime - buttonD7PreviousPressedTime;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	//buttonD7PreviousPressedTime = buttonD7PressedTime;
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	//tOutputResult.restartDelayed();
	_PM(btnPins[btnOnISR]); _PP(": "); _PL(btnDurationBetweenPresses[btnOnISR]);
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "%d", btnDurationBetweenPresses[btnOnISR]);
		//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
		mqttClient.publish("flowmeter/flow/0", msg);
		_PM("flowmeter/flow/0 = "); _PL(msg); _PL();
	}
}

void OnButtonReleased()
{
	_PN("tButtonReleased:OnButtonReleased\n");
	//attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, RISING);
	attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, RISING);
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

void OutputResult()
{
	//Serial.print(millis()); Serial.print(":"); Serial.println(buttonD7TimeSlot);
	//_PM(buttonD7TimeSlot);
}

void setup()
{
	Serial.begin(74880);
	delay(100);
	_PM("Starting application...");

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

	mqttClient.setServer(MQTT_SERVER, 1883);
	tConnctMQTT.enable();
	tButtonReleased.enable();
}

void loop()
{
	mqttClient.loop();
	ts.execute();
}
