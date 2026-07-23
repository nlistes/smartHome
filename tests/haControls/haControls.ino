#include <OneWire.h>
#include <DallasTemperature.h>

// ==== Test options ==================
#define _APP_TEST_
//#define _WIFI_TEST_
#define _MQTT_TEST_

// ==== Debug options ==================
#define _DEBUG_
#define _DEBUG_SYSTEM_
#define _DEBUG_INTERNAL_
#define _DEBUG_EXTERNAL_

// ==== Host parameters ===============
#define SOFTWARE_VERSION "20260722-01"
#define HOSTNAME "haControls"
#define DEVICE_TYPE "haTest"
#define DEVICE_NAME "controls"

#ifdef _WIFI_TEST_
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#else
#define PRIMARY_SSID "OSIS"
#define PRIMARY_PASS "IBMThinkPad0IBMThinkPad1"
#endif // _WIFI_TEST_

#include "../../templates/espTask/src/espTask.inc"

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

#include "../../templates/espHA/src/espHA.inc"

#ifdef _APP_TEST_

#define DATA_GET_INTERVAL	20
uint16_t test_value = 0;
HASensorNumber Counter("Counter");
HASensorNumber RSSI("RSSI");

HASwitch led("led");

void onSwitchCommand(bool state, HASwitch* sender)
{
	digitalWrite(LED_BUILTIN, (state ? HIGH : LOW));
	sender->setState(state); // report state back to the Home Assistant
}


void onGetTestValue()
{
	test_value++;
	_I_PMP(F("Test counter: ")); _I_PL(test_value);
	Counter.setValue(test_value);
	RSSI.setValue(WiFi.RSSI());
	//led.setState(true); // use any state you want
}
Task taskGetTestValue(DATA_GET_INTERVAL* TASK_SECOND, TASK_FOREVER, &onGetTestValue, &ts);

#endif // _APP_TEST_



void setup()
{
#include "../../templates/espTask/src/espTask_setup.inc"

#include "../../templates/espHA/src/espHA_setup.inc"

#ifdef _APP_TEST_
	// HA Sensors
	Counter.setIcon("mdi:home");
	Counter.setName("Counter");
	Counter.setUnitOfMeasurement("x");

	RSSI.setIcon("mdi:wifi");
	RSSI.setName("RSSI");
	RSSI.setUnitOfMeasurement("db");

	led.setIcon("mdi:lightbulb");
	led.setName("My LED");
	led.onCommand(onSwitchCommand);


	taskGetTestValue.enableDelayed();
#endif // _APP_TEST_

}

void loop()
{
	ts.execute();
}
