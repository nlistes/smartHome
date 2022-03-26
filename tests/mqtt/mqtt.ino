// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PX(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PP(a)
#define _PL(a)
#define _PX(a)
#endif

#include <Bounce2.h>
#include <ESP8266WiFi.h>
//#include <Ethernet.h>
#include <PubSubClient.h>

#define SSID "OSIS"
#define PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_SERVER "10.20.30.60"

WiFiClient ethClient;
//EthernetClient ethClient;
//byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
PubSubClient mqttClient(ethClient);

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

unsigned long lastMsg = 0;
int value = 0;

void callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if ((char)payload[0] == '1')
    {
        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Active low on the ESP-01)
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    }

}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
    Serial.begin(74880);
    delay(100);
    _PL(); _PP("Connecting to "); _PL(SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        _PP(".");
    }

    randomSeed(micros());

    _PL(); _PL("WiFi connected!");
    _PP("IP address: "); _PL(WiFi.localIP());

    mqttClient.setServer(MQTT_SERVER, 1883);
    mqttClient.setCallback(callback);
}

void loop()
{

    if (!mqttClient.connected())
    {
        reconnect();
    }
    mqttClient.loop();

    unsigned long now = millis();
    if (now - lastMsg > 10000)
    {
        lastMsg = now;
        ++value;
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        mqttClient.publish("outTopic", msg);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            mqttClient.publish("outTopic", "hello world");
            // ... and resubscribe
            mqttClient.subscribe("inTopic");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

