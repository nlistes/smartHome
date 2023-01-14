// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_
//#define _MQTT_TEST_
#define _WIFI_TEST_

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

#define CONNECTION_TIMEOUT 10

#if defined(ARDUINO_ARCH_ESP8266) || (defined ARDUINO_ARCH_ESP32)
void OnConnectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		_PL(""); _PMP("Connecting to "); _PP(PRIMARY_SSID);
		WiFi.mode(WIFI_STA);
		WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);

		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			_PP(".");
		}

		_PL(); _PML("CONNECTED!");
		_PMP("IP address: "); _PL(WiFi.localIP());
		//_PL(WiFi.localIP().toString);

		// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
		WiFi.setAutoReconnect(true);
		WiFi.persistent(true);
	}
}
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);
#endif


#ifdef _MQTT_TEST_
#define MQTT_SERVER "10.20.30.60"
#define MQTT_CLIENT_NAME "espTask_test"
#else
#define MQTT_SERVER "10.20.30.70"
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
		_PL();  _PMP("Connecting to MQTT... ");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
		{
			_PL(" connected!");
			mqttClient.subscribe("tempmeter/boiler/valve");
		}
		else
		{
			_PP("failed, rc="); _PL(mqttClient.state());
		}
	}

}
Task tConnectMQTT(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnConnectMQTT, &ts);

#define MOTOR_PIN 23

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println();

	if ((char)payload[0] == '1')
	{
		digitalWrite(MOTOR_PIN, HIGH);   // Turn the LED on (Active low on the ESP-01)
	}
	else
	{
		digitalWrite(MOTOR_PIN, LOW);  // Turn the LED off by making the voltage HIGH
	}

}


#ifdef _TEST_
void OnSendTest()
{
	if (mqttClient.connected())
	{
		snprintf(msg, MSG_BUFFER_SIZE, "test");
		snprintf(topic, TOPIC_BUFFER_SIZE, "test/test");
		mqttClient.publish(topic, msg);
		_PMP(topic); _PP(" = ");  _PL(msg);
	}
}
Task tSendTest(5 * TASK_SECOND, TASK_FOREVER, &OnSendTest, &ts);
#endif // _TEST_



void setup()
{
	delay(5000);
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

	mqttClient.setServer(MQTT_SERVER, 1883);
	mqttClient.setCallback(mqtt_callback);

	tConnectMQTT.enable();

#ifdef _TEST_
	tSendTest.enable();
#endif // _TEST_

	pinMode(MOTOR_PIN, OUTPUT);

}

void loop()
{
	mqttClient.loop();
	ts.execute();
}
