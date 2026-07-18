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
