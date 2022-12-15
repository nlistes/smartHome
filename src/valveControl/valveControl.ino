// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_
#define _MQTT_TEST_
#define _WIFI_TEST_

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

#if defined (ARDUINO_ARCH_AVR)
#define COM_SPEED 9600
#endif

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

#if defined (ARDUINO_ARCH_AVR)
#include <SPI.h>
#include <Ethernet.h>
#endif

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

#define CONNECTION_TIMEOUT 15

#if defined(ARDUINO_ARCH_ESP8266) || (defined ARDUINO_ARCH_ESP32)
void OnConnectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		_PM("Connecting to "); _PP(PRIMARY_SSID);
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
}
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnConnectWiFi, &ts);
#endif


#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "valveControl_test"
#else
#define MQTT_SERVER "10.20.30.71"
#define MQTT_CLIENT_NAME "valveControl"
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
		mqttClient.setServer(MQTT_SERVER, 1883);
		_PL();  _PM("Attempting MQTT connection... ");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
		{
			_PL(" connected!");
			mqttClient.subscribe("inTopic");
		}
		else
		{
			_PL("");  _PM("failed, rc="); _PL(mqttClient.state());
		}
	}
}
Task tConnectMQTT(CONNECTION_TIMEOUT * TASK_SECOND, TASK_FOREVER, &OnConnectMQTT, &ts);


void setup()
{
	Serial.begin(COM_SPEED);
	delay(100);
	_PL("Programm started...");

#if defined (ARDUINO_ARCH_AVR)
	Ethernet.begin(MAC_ADDRESS);
	_PL("Newtwork connected");
	_PP("IP address: "); _PL(Ethernet.localIP());
#endif

#if defined(ARDUINO_ARCH_ESP8266) || (defined ARDUINO_ARCH_ESP32)
	tConnectWiFi.enable();
#endif

	_PL("");
	tConnectMQTT.enable();

	mqttClient.setServer(MQTT_SERVER, 1883);
	//mqttClient.setCallback(callback);
}

void loop()
{
	mqttClient.loop();
	ts.execute();
}
