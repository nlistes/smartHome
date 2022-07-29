// ==== Debug and Test options ==================
#include <SensorModbusMaster.h>
#include <Arduino.h>
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
//HardwareSerial modbusSerial = Serial1;
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

byte command[8] = { 0x01, 0x03, 0x05, 0xBB, 0x00, 0x02, 0xB4, 0xE2 };

void OnConnectWiFi();
Task tConnectWiFi(CONNECTION_TIMEOUT* TASK_SECOND, TASK_ONCE, &OnConnectWiFi, &ts);

void OnSendData();
Task tSendData(CONNECTION_TIMEOUT* TASK_SECOND, TASK_FOREVER, &OnSendData, &ts);

long modbusBaudRate = 9600; // The baud rate the sensor uses
byte modbusAddress = 0x01;

modbusMaster modbus;

void OnSendData()
{
	_PM("Flow: "); _PL(modbus.float32FromRegister(0x03, 1446, littleEndian));
}

void setup()
{
	Serial.begin(COM_SPEED);
	_PL(); _PN("Starting application...");

	Serial2.begin(modbusBaudRate);
	modbus.begin(modbusAddress, Serial2);
	//modbus.setDebugStream(&Serial);
	_PM("ModBus device address check. Value must be [361]: "); _PL(modbus.float32FromRegister(0x03, 360, littleEndian));
	_PM("ModBus device software version: "); _PP(modbus.StringFromRegister(0x03, 1467, 2)); _PP("."); _PL(modbus.StringFromRegister(0x03, 1468, 2));
	_PM("Current instantaneous flow unit: "); _PL(modbus.int16FromRegister(0x03, 1437 - 1, littleEndian));
	_PM("Current cumulative flow unit: "); _PL(modbus.int16FromRegister(0x03, 1438 - 1, littleEndian));
	_PM("Current cumulative flow decimal point position: "); _PL(modbus.int16FromRegister(0x03, 1439 - 1, littleEndian));
	_PM("Not required (the decimal point position for cumulative heat)
		: "); _PL(modbus.int16FromRegister(0x03, 1440 - 1, littleEndian));
	//tConnectWiFi.enable();
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

