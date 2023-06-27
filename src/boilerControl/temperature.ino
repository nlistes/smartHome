// ### TEMPERATURE START PROCEDURES ###

void onPrepareTemperature()
{
	_I_PL();  _I_PML("Requesting temperatures... ");
	temperatureSensors.requestTemperatures();
	taskGetTempereature.restartDelayed(750 * TASK_MILLISECOND);
}

void onGetTempereature()
{
	_I_PML("Geting temperatures... ");

	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		boilerThermometers[i].value = temperatureSensors.getTempC(boilerThermometers[i].id);
	}
}

void onShowTemperature()
{
	for (int i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
	{
		_I_PMP("Temperature [");  _I_PP(boilerThermometers[i].name); _I_PP("] = "); _I_PL(boilerThermometers[i].value);
	}
}

void onSendTemperature()
{
	onConnectMQTT();
	if (mqttClient.connected())
	{
		for (uint8_t i = 0; i < ACTUAL_TEMPERATURE_SENSORS; i++)
			if (boilerThermometers[i].value > 0)
			{
				snprintf(msg, MSG_BUFFER_SIZE, "%2.2f", boilerThermometers[i].value);
				snprintf(topic, TOPIC_BUFFER_SIZE, "boiler/pagrabs/temp-%s", boilerThermometers[i].name);
				mqttClient.publish(topic, msg);
				_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
			}
	}
}
// ### TEMPERATURE END PROCEDURES ###

