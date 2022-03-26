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
#define BTN_REPEAT    5000 //1000
#define BNT_RPT_CNT   5
#define BTN_RAPID     250
#define BNT_DEBOUNCE  5 //20

#define TI_INTERVAL   15000 //500 //button press delay



Scheduler ts;
void OnButtonPressed();
void interruptCallback();

Task tButtonPressed(BTN_REPEAT, TASK_FOREVER, &OnButtonPressed, &ts);
Task tInterrupt(TI_INTERVAL, TASK_ONCE, &interruptCallback, &ts);

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
	if (pressCnt == 0)
	{
		attachInterrupt(digitalPinToInterrupt(BTN_SEL_PIN), &releaseButtonISR, FALLING);
		attachInterrupt(digitalPinToInterrupt(BTN_PLUS_PIN), &releaseButtonISR, FALLING);
		attachInterrupt(digitalPinToInterrupt(BTN_MINUS_PIN), &releaseButtonISR, FALLING);
		tButtonPressed.setInterval(BTN_REPEAT);
		pressCnt++;
	}

	if (pressCnt == BNT_RPT_CNT)
	{
		tButtonPressed.setInterval(BTN_RAPID);
	}

	//if (!panel_status)
	//{
	//	_PL("Panel On");
	//	switchDisplayNow((night_time ? DEMPTY : DHUMIDITY), 0);
	//}
	//else
	//{
	//	onButtonPressed();
	//}

	onButtonPressed();

}


void onButtonPressed()
{
	bool sel, plus, minus;
	int value, increment = 0;
	time_t tnow;

	_PM("procedure:onButtonPressed");

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

/*
	//if (sel)
	//{
	//    switch (displayNow) {
	//    case DHUMIDITY:
	//    case DEMPTY:
	//        switchDisplayNow(DSET, 0);

	//        measurePower(false);

	//        ts.disableAll();
	//        tDisplay.enable();
	//        tIsr.enableDelayed();

	//        tSettingsTimeout.set(TSETTOUT_INTERVAL * TASK_SECOND, 1, &settingsToutCallback);
	//        tSettingsTimeout.enableDelayed();

	//        adjParam = &parameters.high;
	//        paramIndex = PARAMIDXSTART;
	//        break;

	//    case DSET:
	//        switchDisplayNow(DSET_UL, 0);
	//        break;

	//    case DCLK:
	//        tnow = myTZ.toLocal(now());
	//        calcTime(tnow);
	//        adjParam = &mytime.cn;
	//        paramIndex = CLOCKIDXSTART;
	//        switchDisplayNow(DCLK_CN, 0);
	//        break;

	//    case DLOG:
	//        paramIndex = 0;
	//        readLogEntry(paramIndex);
	//        switchDisplayNow(DLOG_V, 0);
	//        break;

	//    case DFRN:
	//        tWater.setInterval(parameters.saturate * TASK_MINUTE);
	//        tWater.setIterations(parameters.retries + 1);
	//        tWater.enable();
	//        tMeasure.set(TMEASURE_WATERTIME * TASK_SECOND, -1, &measureCallback);
	//        tMeasure.enableDelayed();
	//        error = false;
	//        settingsDoTout();
	//        switchDisplayNow(DCLK_SET, 0);
	//        return;
	//        break;


	//    case DSET_UL:
	//    case DSET_LL:
	//    case DSET_PT:
	//    case DSET_RT:
	//    case DSET_ST:
	//    case DSET_SL:
	//    case DSET_UP:
	//        switchDisplayNow(displayNow + 1, 0);
	//        adjParam++;
	//        paramIndex++;
	//        break;

	//    case DLOG_V:
	//    case DLOG_P:
	//        displayOnlyDoTout();
	//        tMeasure.enableDelayed();
	//        switchDisplayNow(DHUMIDITY, 0);
	//        return;

	//    case DSET_AD:
	//        saveParameters();
	//        settingsDoTout();
	//        switchDisplayNow(DSTORED, 0);
	//        return;

	//    case DCLK_CN:
	//    case DCLK_YR:
	//    case DCLK_MT:
	//    case DCLK_DY:
	//    case DCLK_HR:
	//        switchDisplayNow(displayNow + 1, 0);
	//        adjParam++;
	//        paramIndex++;
	//        break;

	//    case DCLK_MN:
	//        setTimeNow();
	//        settingsDoTout();
	//        switchDisplayNow(DCLK_SET, 0);
	//        return;
	//    }
	//}
*/
/*
	//if (plus || minus)
	//{
	//    switch (displayNow) {
	//    case DHUMIDITY:
	//    case DEMPTY:
	//        if (plus) {
	//            switchDisplayNow(DTIME, 0);
	//        }
	//        else {
	//            if (logCount) switchDisplayNow(DRUN, 0);
	//        }
	//        return;
	//        break;

	//    case DSET:
	//        switchDisplayNow(DCLK, 0);
	//        break;

	//    case DCLK:
	//        switchDisplayNow(DFRN, 0);
	//        break;

	//    case DFRN:
	//        if (logCount) switchDisplayNow(DLOG, 0);
	//        else switchDisplayNow(DSET, 0);
	//        break;

	//    case DLOG:
	//        switchDisplayNow(DSET, 0);
	//        break;

	//    case DLOG_V:
	//    case DLOG_P:
	//        paramIndex += increment;
	//        if (paramIndex < 0) paramIndex = logCount;
	//        if (paramIndex >= logCount) paramIndex = 0;
	//        readLogEntry(paramIndex);
	//        switchDisplayNow(DLOG_V, 0);
	//        break;

	//    case DSET_UL:
	//    case DSET_LL:
	//    case DSET_PT:
	//    case DSET_ST:
	//    case DSET_RT:
	//    case DSET_SL:
	//    case DSET_UP:
	//    case DSET_AD:
	//    case DCLK_CN:
	//    case DCLK_YR:
	//    case DCLK_MT:
	//    case DCLK_DY:
	//    case DCLK_HR:
	//    case DCLK_MN:
	//        if (displayNow > DCLK) {
	//            switch (mytime.mt) {
	//            case 2:
	//                paramMax[DAYSIDX] = isLeapYear(mytime.cn * 100 + mytime.yr) ? 29 : 28;
	//                break;

	//            case 4:
	//            case 6:
	//            case 9:
	//            case 11:
	//                paramMax[DAYSIDX] = 30;
	//                break;

	//            default:
	//                paramMax[DAYSIDX] = 31;
	//            }
	//        }
	//        value = (int)*adjParam + increment;
	//        if (value > (int)paramMax[paramIndex]) value = (int)paramMin[paramIndex];
	//        if (value < (int)paramMin[paramIndex]) value = (int)paramMax[paramIndex];
	//        *adjParam = (byte)value;
	//        // Special case checks:
	//        if (parameters.high <= parameters.low) parameters.high = parameters.low + 1;
	//        if (parameters.wakeup >= parameters.gotosleep) parameters.gotosleep = parameters.wakeup + 1;
	//        switchDisplayNow(displayNow, 0);
	//    }
	//}
*/

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
	ts.execute();
}
