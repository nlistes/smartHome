// BEGIN TEMPLATE [HA] - SETUP

	mqttClient.setDataPrefix("ha_in");

	haDevice.setName(HOSTNAME);
	haDevice.setModel(DEVICE_TYPE);
	haDevice.setSoftwareVersion(SOFTWARE_VERSION);
	haDevice.setManufacturer("SelfMade Ltd.");

	mqttClient.begin(MQTT_SERVER, MQTT_USER_NAME, MQTT_PASSWORD);
	taskRunMQTT.enable();

// END TEMPLATE [HA] - SETUP
