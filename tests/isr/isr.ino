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
#define BTN_PIN 26
#endif

#include <TaskScheduler.h>

#define BNT_DEBOUNCE  5 //20

Scheduler runner;

volatile int isrCount = 0, count = 0;

void OnButtonPressed();
Task tButtonPressed(TASK_IMMEDIATE, TASK_ONCE, &OnButtonPressed, &runner);

void OnButtonReleased();
Task tButtonReleased(TASK_IMMEDIATE, TASK_ONCE, &OnButtonReleased, &runner);

void printCount();
Task tPrintCount(5 * TASK_SECOND, TASK_FOREVER, &printCount, &runner);


void IRAM_ATTR pressButtonISR_0()
{
	detachInterrupt(digitalPinToInterrupt(BTN_PIN));
	tButtonPressed.restartDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR()
{
	detachInterrupt(digitalPinToInterrupt(BTN_PIN));
	tButtonReleased.restartDelayed(BNT_DEBOUNCE);

}

void OnButtonPressed()
{
	isrCount++;
	_PN("tButtonPressed:OnButtonPressed");
	attachInterrupt(digitalPinToInterrupt(BTN_PIN), releaseButtonISR, RISING);
}

void OnButtonReleased()
{
	_PN("tButtonReleased:OnButtonReleased\n");
	attachInterrupt(digitalPinToInterrupt(BTN_PIN), pressButtonISR_0, FALLING);

}

void setup()
{
	Serial.begin(COM_SPEED);
	delay(1000);
	_PL(); _PN("Starting application...");

	pinMode(BTN_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BTN_PIN), pressButtonISR_0, FALLING);
	//tButtonReleased.enable();
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
