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

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#define COM_SPEED 74880
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#define COM_SPEED 115200
#endif

#include <TaskScheduler.h>

#ifdef _TEST_
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#else
#define PRIMARY_SSID "PAGRABS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _TEST_

#define CONNECTION_TIMEOUT 5

Scheduler ts;

byte command[8] = { 0x01, 0x03, 0x05, 0xBB, 0x00, 0x02, 0xB4, 0xE2 }

void OnConnectWiFi();
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);

void OnSendData();
Task tSendData(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnSendData, &ts);

void OnSendData()
{
	Serial1.write(command, 8);
}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PL(); _PM("Starting application...");

	Serial1.begin(9600);
	delay(1000);
	//Serial1.println("");
	Serial1.println("1");

	tConnectWiFi.enable();
	//tSendData.enableDelayed();
}

void loop()
{
	ts.execute();
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

