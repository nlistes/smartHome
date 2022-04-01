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

#define BTN_SEL_PIN   D7
#define BTN_PLUS_PIN  D6
#define BTN_MINUS_PIN D5

#define BTN_INIT      20000 //2000
#define BNT_DEBOUNCE  5 //20

#define TI_INTERVAL   15000 //500 //button press delay



Scheduler runner;
void OnButtonPressed();
void interruptCallback();

Task tButtonPressed(BTN_INIT, TASK_FOREVER, &OnButtonPressed, &runner);
Task tInterrupt(TI_INTERVAL, TASK_ONCE, &interruptCallback, &runner);

int pressCnt = 0;
void IRAM_ATTR pressButtonISR()	// attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
{
	detachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN));
	detachInterrupt(digitalPinToInterrupt(BTN_PLUS_PIN));
	detachInterrupt(digitalPinToInterrupt(BTN_MINUS_PIN));
	_PM("pressButtonISR");
	tButtonPressed.setInterval(BTN_INIT);
	tButtonPressed.enableDelayed(BNT_DEBOUNCE);
}

void IRAM_ATTR releaseButtonISR()
{
	detachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN));
	detachInterrupt(digitalPinToInterrupt(BTN_PLUS_PIN));
	detachInterrupt(digitalPinToInterrupt(BTN_MINUS_PIN));
	_PM("releaseButtonISR");
	tButtonPressed.disable();
	tInterrupt.restartDelayed(BNT_DEBOUNCE);
}

void interruptCallback()
{
	_PM("tInterrupt:interruptCallback\n");
	attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &pressButtonISR, RISING);
	attachInterrupt(digitalPinToInterrupt(BTN_PLUS_PIN), &pressButtonISR, RISING);
	attachInterrupt(digitalPinToInterrupt(BTN_MINUS_PIN), &pressButtonISR, RISING);
	pressCnt = 0;
}


void OnButtonPressed()
{
	_PM("tButtonPressed:OnButtonPressed");

	bool sel, plus, minus;
	int value, increment = 0;
	time_t tnow;


	sel = plus = minus = false;
	sel = digitalRead(BTN_SEL_PIN);

	if (!sel)
	{
		plus = digitalRead(BTN_PLUS_PIN);
		minus = digitalRead(BTN_MINUS_PIN);

		if (plus && minus)
		{
			plus = minus = false;
		}
	}

	_PP("\tsel/plus/minus="); _PP(sel); _PP(plus); _PL(minus);
	_PP("\trepeat Count="); _PL(pressCnt);

	if (plus) increment = 1;
	if (minus) increment = (-1);


	if (sel || plus || minus)
	{
		_PL("\tsome button pressed");
		pressCnt++;
	}
	else
	{
		_PL("\t\t\t\t\t!!! no buttons pressed !!!");
		tButtonPressed.disable();
		tInterrupt.restart();
	}

}

void setup()
{
	Serial.begin(74880);
	_PM("Starting...");
	//wdt_disable();
	//delay(50);
	tInterrupt.enable();
}

void loop()
{
	runner.execute();
}
