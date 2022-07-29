// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PN(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
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
//#if defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_AVR)
    #include <Ethernet.h>
#elif defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
#endif

#include <PubSubClient.h>
#include <TaskScheduler.h>


#define SSID "OSIS"
#define PASS "IBMThinkPad0IBMThinkPad1"
#define MQTT_SERVER "10.20.30.60"

//EthernetClient ethClient;
//byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

Scheduler runner;

#define MQTT_SEND_PERIOD 10
void mqttSendData();
Task tmqttSendData(MQTT_SEND_PERIOD * TASK_SECOND, TASK_FOREVER, &mqttSendData, &runner);

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
        //digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Active low on the ESP-01)
    }
    else
    {
        //digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    }

}

void setup()
{
    //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
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
    tmqttSendData.enable();
}

void loop()
{

    runner.execute();
    mqttClient.loop();
}

void mqttSendData()
{
    if (mqttClient.connected())
    {
        ++value;
        //snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        _PN("Publish message: "); _PL(msg);
        mqttClient.publish("outTopic", msg);
    }
    else
    {
        _PN("Attempting MQTT connection... ");
        if (mqttClient.connect("cityHeatControl"))
        {
            _PL("connected");
            mqttClient.subscribe("inTopic");
        }
        else
        {
            _PP("failed, rc="); _PL(mqttClient.state());
        }
    }
}
