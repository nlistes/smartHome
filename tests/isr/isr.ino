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

#include <TaskScheduler.h>

Scheduler runner;

#define BTN_PIN   D6

volatile int isrCount = 0, count = 0;

void IRAM_ATTR btnISR()
{
	isrCount++;
}

void printCount();
Task tPrintCount(5* TASK_SECOND, TASK_FOREVER, &printCount, &runner);


void setup()
{
	Serial.begin(74880);
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
		_PM(isrCount);
		count = isrCount;
	}
}
