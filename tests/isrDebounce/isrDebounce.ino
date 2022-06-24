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

void IRAM_ATTR pressButtonISR_0()
{
	isrAction(0);
}
void OnButtonPressed();
Task tButtonPressed(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed, &runner);

void OnButtonReleased();
Task tButtonReleased(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased, &runner);


void printCount();
Task tPrintCount(5 * TASK_SECOND, TASK_FOREVER, &printCount, &runner);

void OnButtonPressed()
{
	_PN("tButtonPressed:OnButtonPressed");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &releaseButtonISR, FALLING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[1]), &releaseButtonISR, FALLING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[2]), &releaseButtonISR, FALLING);
}

void OnButtonReleased()
{
	_PN("tButtonReleased:OnButtonReleased\n");
	attachInterrupt(digitalPinToInterrupt(btnPins[0]), &pressButtonISR_0, RISING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[1]), &pressButtonISR_1, RISING);
	//attachInterrupt(digitalPinToInterrupt(btnPins[2]), &pressButtonISR_2, RISING);
}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PL(); _PN("Starting application...");

	pinMode(BTN_PIN, INPUT);
	//attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnISR, RISING);
	//tPrintCount.enable();
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
