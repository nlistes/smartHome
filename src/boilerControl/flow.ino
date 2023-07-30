// ### FLOW START PROCEDURES ###

void IRAM_ATTR pressButtonISR_0()
{
	int8_t btnOnISR = 0;
#ifdef USE_FLOW_STRUCT
	pressTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin));
#else
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
#endif // USE_FLOW_STRUCT
	taskButtonPressed_0.restartDelayed(FLOW_COUNTER_DEBOUNCE_TIME);
}

void IRAM_ATTR pressButtonISR_1()
{
	int8_t btnOnISR = 1;
#ifdef USE_FLOW_STRUCT
	pressTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin));
#else
	btnPressedTime[btnOnISR] = millis();
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
#endif
	taskButtonPressed_1.restartDelayed(FLOW_COUNTER_DEBOUNCE_TIME);
}

void IRAM_ATTR releaseButtonISR_0()
{
	int8_t btnOnISR = 0;
#ifdef USE_FLOW_STRUCT
	detachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin));
#else
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
#endif
	taskButtonReleased_0.restartDelayed(FLOW_COUNTER_DEBOUNCE_TIME);
}

void IRAM_ATTR releaseButtonISR_1()
{
	int8_t btnOnISR = 1;
#ifdef USE_FLOW_STRUCT
	detachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin));
#else
	detachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]));
#endif
	taskButtonReleased_1.restartDelayed(FLOW_COUNTER_DEBOUNCE_TIME);
}

void onButtonPressed_0()
{
	int8_t btnOnISR = 0;
	_I_PL();
	_I_PML("OnButtonPressed_0");
#ifdef USE_FLOW_STRUCT
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin), &releaseButtonISR_0, FALLING);
	boilerFlowCounters[btnOnISR].value++;
	boilerFlowCounters[btnOnISR].durationBetweenPress = pressTime[btnOnISR] - boilerFlowCounters[btnOnISR].previousPressTime;
	boilerFlowCounters[btnOnISR].flow = (60 * 60 * 1000) / (boilerFlowCounters[btnOnISR].durationBetweenPress);
	boilerFlowCounters[btnOnISR].previousPressTime = pressTime[btnOnISR];
	_I_PMP(boilerFlowCounters[btnOnISR].value); _I_PP(": "); _I_PP(boilerFlowCounters[btnOnISR].durationBetweenPress); _I_PP(": "); _I_PL(boilerFlowCounters[btnOnISR].flow);
#else
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_0, FALLING);
	flowMeterValue[btnOnISR]++;
	flowMeterValueUsed[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
#endif
}

void onButtonPressed_1()
{
	int8_t btnOnISR = 1;
	_I_PL();
	_I_PML("OnButtonPressed_1");
#ifdef USE_FLOW_STRUCT
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin), &releaseButtonISR_0, FALLING);
	boilerFlowCounters[btnOnISR].value++;
	boilerFlowCounters[btnOnISR].durationBetweenPress = pressTime[btnOnISR] - boilerFlowCounters[btnOnISR].previousPressTime;
	boilerFlowCounters[btnOnISR].flow = (60 * 60 * 1000) / (boilerFlowCounters[btnOnISR].durationBetweenPress);
	boilerFlowCounters[btnOnISR].previousPressTime = pressTime[btnOnISR];
	_I_PMP(boilerFlowCounters[btnOnISR].value); _I_PP(": "); _I_PP(boilerFlowCounters[btnOnISR].durationBetweenPress); _I_PP(": "); _I_PL(boilerFlowCounters[btnOnISR].flow);
#else
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &releaseButtonISR_1, FALLING);
	flowMeterValue[btnOnISR]++;
	flowMeterValueUsed[btnOnISR]++;
	btnDurationBetweenPresses[btnOnISR] = btnPressedTime[btnOnISR] - btnPreviousPressedTime[btnOnISR];
	flowSpeed[btnOnISR] = (60 * 60 * 1000) / (btnDurationBetweenPresses[btnOnISR]);
	btnPreviousPressedTime[btnOnISR] = btnPressedTime[btnOnISR];
	_I_PMP(flowMeterValue[btnOnISR]); _I_PP(": "); _I_PP(btnDurationBetweenPresses[btnOnISR]); _I_PP(": "); _I_PL(flowSpeed[btnOnISR]);
#endif
}

void onButtonReleased_0()
{
	int8_t btnOnISR = 0;
	_I_PL();
	_I_PML("OnButtonReleased_0");
#ifdef USE_FLOW_STRUCT
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin), &pressButtonISR_0, RISING);
#else
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_0, RISING);
#endif
}

void onButtonReleased_1()
{
	int8_t btnOnISR = 1;
	_I_PL();
	_I_PML("OnButtonReleased_1");
#ifdef USE_FLOW_STRUCT
	attachInterrupt(digitalPinToInterrupt(boilerFlowCounters[btnOnISR].pin), &pressButtonISR_1, RISING);
#else
	attachInterrupt(digitalPinToInterrupt(btnPins[btnOnISR]), &pressButtonISR_1, RISING);
#endif
}

void onFlowCorrection()
{
	for (int i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
#ifdef USE_FLOW_STRUCT
		if ((millis() - boilerFlowCounters[i].previousPressTime) > boilerFlowCounters[i].durationBetweenPress & boilerFlowCounters[i].previousPressTime != 0)
		{
			boilerFlowCounters[i].flow = (60 * 60 * 1000) / (millis() - boilerFlowCounters[i].previousPressTime);
		}
#else
		if ((millis() - btnPreviousPressedTime[i]) > btnDurationBetweenPresses[i] & btnPreviousPressedTime[i] != 0)
		{
			flowSpeed[i] = (60 * 60 * 1000) / (millis() - btnPreviousPressedTime[i]);
		}
#endif
}

void onShowFlow()
{
	_I_PL();
	for (int i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
	{
#ifdef USE_FLOW_STRUCT
		_I_PMP("flowMeter"); _I_PP("["); _I_PP(boilerFlowCounters[i].name); _I_PP("] flow: "); _I_PP(boilerFlowCounters[i].flow); _I_PP(" value: "); _I_PL(boilerFlowCounters[i].value);
#else
		_I_PMP("flowMeter"); _I_PP("["); _I_PP(flowMeterName[i]); _I_PP("] flow: "); _I_PP(flowSpeed[i]); _I_PP(" value: "); _I_PL(flowMeterValue[i]);
#endif
	}
}

void onSendFlow()
{
	if (mqttClient.connected())
	{
		_E_PL();
		for (uint8_t i = 0; i < ACTUAL_FLOW_COUNTERS; i++)
		{
			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowSpeed[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%s/%s-flow", DEVICE_TYPE, DEVICE_NAME, flowMeterName[i]);
			//itoa(btnDurationBetweenPresses[btnOnISR], msg, 10);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValue[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%s/%s-value", DEVICE_TYPE, DEVICE_NAME, flowMeterName[i]);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

			snprintf(msg, MSG_BUFFER_SIZE, "%d", flowMeterValueUsed[i]);
			snprintf(topic, TOPIC_BUFFER_SIZE, "%s/%s/%s-used", DEVICE_TYPE, DEVICE_NAME, flowMeterName[i]);
			mqttClient.publish(topic, msg);
			_E_PMP(topic); _E_PP(" = ");  _E_PL(msg);

		}
	}
}

// ### FLOW END PROCEDURES ###

