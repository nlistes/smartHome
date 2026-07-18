// BEGIN TEMPLATE [HA] - DEFINITIONS
// !!! Do not make changes! Update from espHA.ino
// 1.000 - Basic HA integration
#define TEMPLATE_VERSION "H-1.000"
#include <ArduinoHA.h>

HADevice haDevice(DEVICE_TYPE "-" DEVICE_NAME);
HAMqtt mqttClient(ethClient, haDevice);

void onRunMQTT()
{
	mqttClient.loop();
}
Task taskRunMQTT(TASK_IMMEDIATE, TASK_FOREVER, &onRunMQTT, &ts);

// !!! Do not make changes! Update from espHA.ino
// END TEMPLATE [HA] - DEFINITIONS
