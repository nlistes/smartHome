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

void sendValveStatus()
{
	snprintf(msg, MSG_BUFFER_SIZE, "%u", heatRequired);
	snprintf(topic, TOPIC_BUFFER_SIZE, "boiler/pagrabs/heatRequired");
	mqttClient.publish(topic, msg);
	_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);
}

void onSendValveStatus()
{
	if (heatRequiredChanged)
	{
		sendValveStatus();
	}
}

// ### VALVE START PROCEDURES###
