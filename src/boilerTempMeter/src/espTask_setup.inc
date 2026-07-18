	// BEGIN TEMPLATE - SETUP
	// !!! Do not make changes! Update from espTask.ino
	Serial.begin(COM_SPEED);
	delay(100);
	randomSeed(analogRead(0));
	pinMode(LED_BUILTIN, OUTPUT);

	_S_PL(""); _S_PML(F("Programm started!"));
	_S_PMP(F("Template version: ")); _S_PL(TEMPLATE_VERSION);
	_S_PMP(F("Software version: ")); _S_PL(SOFTWARE_VERSION);

	WiFi.onEvent(onWiFiConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent(onWiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent(onWiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

	WiFi.mode(WIFI_STA);
	WiFi.setHostname(HOSTNAME);
	WiFi.begin(PRIMARY_SSID, PRIMARY_PASS);
	// https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	ArduinoOTA.setHostname(HOSTNAME);

	ArduinoOTA.onStart
	(
		[]()
		{
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = F("sketch");
			else // U_SPIFFS
				type = F("filesystem");
			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.print(F("Start updating ")); Serial.println(type);
		}
	);

	ArduinoOTA.onEnd
	(
		[]()
		{
			Serial.println(F("\nEnd"));
		}
	);

	ArduinoOTA.onProgress
	(
		[](unsigned int progress, unsigned int total)
		{
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		}
	);

	ArduinoOTA.onError
	(
		[](ota_error_t error)
		{
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
			else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
			else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
			else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
			else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
		}
	);

	ArduinoOTA.begin();

	taskHandleOTA.enable();
	taskShowWiFiStatus.enableDelayed();
// !!! Do not make changes! Update from espTask.ino
// END TEMPLATE - SETUP
