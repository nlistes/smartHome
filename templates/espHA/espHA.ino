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

#include "src/espTask.inc"


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

#include "src/espHA.inc"

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
#include "src/espTask_setup.inc"

#include "src/espHA_setup.inc"

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
