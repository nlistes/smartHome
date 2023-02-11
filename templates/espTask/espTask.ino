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
		_S_PL(""); _S_PMP("Connecting to "); _S_PP(PRIMARY_SSID);

		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			_S_PP(".");
		}

		_S_PL(); _S_PML("WIFI CONNECTED!");
		_S_PMP("IP address: "); _S_PL(WiFi.localIP());
	}
}
Task tConnectWiFi(CONNECTION_TIMEOUT * TASK_SECOND, TASK_FOREVER, &OnConnectWiFi, &ts);


#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "espTask_test"
#else
#define MQTT_SERVER "10.20.30.71"
#define MQTT_CLIENT_NAME "espTask"
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
		_S_PL();  _S_PMP("Connecting to MQTT ["); _S_PP(MQTT_SERVER); _S_PP("] ");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
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
Task tConnectMQTT(CONNECTION_TIMEOUT * TASK_SECOND, TASK_ONCE, &OnConnectMQTT, &ts);

void OnRunMQTT()
{
	mqttClient.loop();
}
Task tRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &OnRunMQTT, &ts);

#ifdef _TEST_
#define DATA_SEND_INTERVAL	40
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
		snprintf(topic, TOPIC_BUFFER_SIZE, "test/test");
		mqttClient.publish(topic, msg);
		_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
	}
}
Task tSendTest(DATA_SEND_INTERVAL* TASK_SECOND, TASK_FOREVER, &OnSendTest, &ts);
#endif // _TEST_



void setup()
{
	delay(1000);
	Serial.begin(COM_SPEED);
	delay(100);
	_S_PL(""); _S_PML("Programm started!");

	WiFi.mode(WIFI_STA);
	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	tConnectWiFi.enable();

	mqttClient.setServer(MQTT_SERVER, 1883);
	//mqttClient.setCallback(mqtt_callback);
	tConnectMQTT.enable();

#ifdef _TEST_
	//tGetTestValue.enableDelayed();
	//tSendTest.enableDelayed();
	tGetTestValue.enable();
	tSendTest.enable();
#endif // _TEST_

}

void loop()
{
	ts.execute();
}
