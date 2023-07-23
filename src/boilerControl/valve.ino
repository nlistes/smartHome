// ### VALVE START PROCEDURES###

void onBoilerForceSwitch()
{
	boilerForceSwitch.update();
}

void onReadSwitch()
{
	heatRequiredChangedBySwitch = boilerForceSwitch.rose();
	heatRequired = heatRequiredChangedBySwitch ? !heatRequired : heatRequired;
}

void onCheckFlowThreshold()
{
	heatRequiredChangedByFlow = (flowSpeed[0] < BOILER_OFF_FLOW_THRESHOLD) && heatRequired;
	if (heatRequiredChangedByFlow)
	{
		heatRequired = false;
		taskCheckFlowThreshold.disable();
		flowMeterValue[1] = 0;
	}
}

void onCheckHeatRequiredStatus()
{
	heatRequiredChanged = heatRequiredChangedBySwitch || heatRequiredChangedByMQTT || heatRequiredChangedByFlow;
	if (heatRequiredChanged && heatRequired)
	{
		taskCheckFlowThreshold.enableDelayed(TASK_MINUTE);
	}
	if (heatRequiredChanged)
	{
		taskSendValveStatus.forceNextIteration();
	}

	heatRequiredChangedByMQTT = false;
	heatRequiredChangedByFlow = false;
}

void onRunValve()
{
	digitalWrite(VALVE_PIN, heatRequired);
	digitalWrite(LOCAL_STATUS_PIN, heatRequired);
	digitalWrite(REMOTE_STATUS_PIN, heatRequired);
}

void onShowValveStatus()
{
	if (heatRequiredChanged)
	{
		_I_PMP("heatRequired: "); _I_PL(heatRequired);
	}
}

void onSendValveStatus()
{
	snprintf(msg, MSG_BUFFER_SIZE, "%u", heatRequired);
	snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%s/status-heatRequired", DEVICE_TYPE, DEVICE_NAME);
	mqttClient.publish(topic, msg);
	_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
}

// ### VALVE START PROCEDURES###
