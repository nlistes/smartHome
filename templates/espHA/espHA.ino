#include <OneWire.h>
#include <DallasTemperature.h>

// ==== Test options ==================
#define _APP_TEST_
//#define _WIFI_TEST_
#define _MQTT_TEST_


// ==== Host parameters ===============
#define SOFTWARE_VERSION "20260718-01"
//#define HOSTNAME "Boiler Control"
//#define DEVICE_TYPE "Boileris"
//#define DEVICE_NAME "Pagrabs"

// BEGIN TEMPLATE - DEFINITIONS
// !!! Do not make changes! Update from espTask.ino
// 1.000 - Clear WiFi connection
#define TEMPLATE_VERSION "W-1.000"

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
#define _PMP(a) SerialD.print(millis()); SerialD.print(F(": ")); SerialD.print(a)
#define _PML(a) SerialD.print(millis()); SerialD.print(F(": ")); SerialD.println(a)
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
#define _S_PMP(a) SerialS.print(millis()); SerialS.print(F(": ")); SerialS.print(a)
#define _S_PML(a) SerialS.print(millis()); SerialS.print(F(": ")); SerialS.println(a)
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
#define _I_PMP(a) SerialI.print(millis()); SerialI.print(F(": ")); SerialI.print(a)
#define _I_PML(a) SerialI.print(millis()); SerialI.print(F(": ")); SerialI.println(a)
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
#define _E_PMP(a) SerialE.print(millis()); SerialE.print(F(": ")); SerialE.print(a)
#define _E_PML(a) SerialE.print(millis()); SerialE.print(F(": ")); SerialE.println(a)
#else
#define _E_PP(a)
#define _E_PL(a)
#define _E_PH(a)
#define _E_PMP(a)
#define _E_PML(a)
#endif

#ifndef HOSTNAME
#define HOSTNAME "ESP32-WIFI"
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
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _WIFI_TEST_

#define CONNECTION_TIMEOUT 10

Scheduler ts;

void onWiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, HIGH);
	_S_PMP(F("Connected to: "));  _S_PL(PRIMARY_SSID);
}

void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
	_S_PMP(F("IP address: ")); _S_PL(WiFi.localIP());
}

void onWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	digitalWrite(LED_BUILTIN, LOW);
	//_S_PMP("Disconnected! Reason: "); _S_PL(info.wifi_sta_disconnected.reason);
}

void onHandleOTA()
{
	ArduinoOTA.handle();
}
Task taskHandleOTA(TASK_IMMEDIATE, TASK_FOREVER, &onHandleOTA, &ts);

void onShowWiFiStatus()
{
	_I_PMP(F("RSSI ["));  _I_PP(WiFi.SSID()); _I_PP(F("] = ")); _I_PL(WiFi.RSSI());
}
Task taskShowWiFiStatus(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &onShowWiFiStatus, &ts);

// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE - DEFINITIONS


// BEGIN TEMPLATE [HA] - DEFINITIONS
// !!! Do not make changes! Update from espHA.ino
// 1.000 - Basic HA integration
#define TEMPLATE_VERSION "H-1.000"
#include <ArduinoHA.h>

#ifdef _MQTT_TEST_
	#define MQTT_SERVER "10.20.30.70"
	#define MQTT_CLIENT_NAME "espTest-"
	#define MQTT_USER_NAME "mqtt"
	#define MQTT_PASSWORD "mqtt"
#else
	#define MQTT_SERVER "10.20.30.80"
	#define MQTT_CLIENT_NAME "espTask-"
	#define MQTT_USER_NAME "mqtt"
	#define MQTT_PASSWORD "mqtt"
#endif // _MQTT_TEST_

HADevice haDevice(DEVICE_TYPE "-" DEVICE_NAME);
HAMqtt mqttClient(ethClient, haDevice);

void onRunMQTT()
{
	mqttClient.loop();
}
Task taskRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &onRunMQTT, &ts);

// !!! Do not make changes! Update from espHA.ino
// END TEMPLATE [HA] - DEFINITIONS


#ifdef _APP_TEST_

#define DATA_GET_INTERVAL	20
uint16_t test_value = 0;
HASensorNumber Counter("Counter");
HASensorNumber RSSI("RSSI");

void onGetTestValue()
{
	test_value++;
	_I_PMP(F("Test counter: ")); _I_PL(test_value);
	Counter.setValue(test_value);
	RSSI.setValue(WiFi.RSSI());
}
Task taskGetTestValue(DATA_GET_INTERVAL* TASK_SECOND, TASK_FOREVER, &onGetTestValue, &ts);

#endif // _APP_TEST_



void setup()
{
	// BEGIN TEMPLATE - SETUP
	// !!! Do not make changes! Update from espTask.ino
	Serial.begin(COM_SPEED);
	delay(100);
	randomSeed(analogRead(0));
	pinMode(LED_BUILTIN, OUTPUT);

	_S_PL(""); _S_PML(F("Programm started!"));
	_S_PMP(F("Template version: ")); _S_PL(TEMPLATE_VERSION);
	_S_PMP(F("Software version: ")); _S_PL(SOFTWARE_VERSION);

	WiFi.onEvent(onWiFiConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent(onWiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent(onWiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(HOSTNAME);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	ArduinoOTA.setHostname(HOSTNAME);

	ArduinoOTA.onStart
	(
		[]()
		{
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = F("sketch");
			else // U_SPIFFS
				type = F("filesystem");
			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.print(F("Start updating ")); Serial.println(type);
		}
	);

	ArduinoOTA.onEnd
	(
		[]()
		{
			Serial.println(F("\nEnd"));
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
			if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
			else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
			else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
			else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
			else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
		}
	);

	ArduinoOTA.begin();

	taskHandleOTA.enable();
	taskShowWiFiStatus.enableDelayed();
// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE - SETUP


// BEGIN TEMPLATE [HA] - SETUP
// !!! Do not make changes! Update from espHA.ino

	mqttClient.setDataPrefix("ha_in");

	haDevice.setName(HOSTNAME);
	haDevice.setModel(DEVICE_TYPE);
	haDevice.setSoftwareVersion(SOFTWARE_VERSION);
	haDevice.setManufacturer("SelfMade Ltd.");

	mqttClient.begin(MQTT_SERVER, MQTT_USER_NAME, MQTT_PASSWORD);
	taskRunMQTT.enable();

// !!! Do not make changes! Update from espHA.ino
// END TEMPLATE [HA] - SETUP

#ifdef _APP_TEST_
	// HA Sensors
	Counter.setIcon("mdi:home");
	Counter.setName("Counter");
	Counter.setUnitOfMeasurement("x");

	RSSI.setIcon("mdi:wifi");
	RSSI.setName("RSSI");
	RSSI.setUnitOfMeasurement("db");

	taskGetTestValue.enableDelayed();
#endif // _APP_TEST_

} // setup()

void loop()
{
	ts.execute();
}
