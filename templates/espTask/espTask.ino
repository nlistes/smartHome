#include <OneWire.h>
#include <DallasTemperature.h>

// ==== Test options ==================
#define _APP_TEST_
//#define _WIFI_TEST_

// ==== Host parameters ===============
#define SOFTWARE_VERSION "20260718-01"
//#define HOSTNAME "ESP32-boilerControl"
//#define DEVICE_TYPE "Boiler"
//#define DEVICE_NAME "Pagrabs"

#include "src/espTask.inc"

#ifdef _APP_TEST_

#define DATA_GET_INTERVAL	20
uint16_t test_value = 0;

void onGetTestValue()
{
	test_value++;
	_I_PMP(F("Test counter: ")); _I_PL(test_value);
}
Task taskGetTestValue(DATA_GET_INTERVAL* TASK_SECOND, TASK_FOREVER, &onGetTestValue, &ts);

#endif // _APP_TEST_



void setup()
{
#include "src/espTask_setup.inc"

#ifdef _APP_TEST_
	taskGetTestValue.enableDelayed();
#endif // _APP_TEST_
}

void loop()
{
	ts.execute();
}
