// ==== Debug and Test options ==================
#define _DEBUG_
#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
#define _PN(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PH(a) SerialD.print(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#define COM_SPEED 74880
#define BTN_PIN D6
#elif defined(ARDUINO_ARCH_ESP32)
#define COM_SPEED 115200
#define BTN_PIN 19
#endif

#include <TaskScheduler.h>

Scheduler runner;

volatile int isrCount = 0, count = 0;

void IRAM_ATTR btnISR()
{
	isrCount++;
}

void printCount();
Task tPrintCount(5* TASK_SECOND, TASK_FOREVER, &printCount, &runner);


void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PL(); _PN("Starting application...");

	pinMode(BTN_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnISR, RISING);
	tPrintCount.enable();
}

void loop()
{
	runner.execute();
}


void printCount()
{
	if (isrCount != count)
	{
		_PM(isrCount - count); _PP(" : "); _PL(isrCount);
		count = isrCount;
	}
}
