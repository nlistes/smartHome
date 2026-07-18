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
