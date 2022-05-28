// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.print(a)
#define _PN(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PH(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PN(a)
#define _PP(a)
#define _PL(a)
#define _PH(a)
#endif

//#include <stdlib.h>
//#include <stdio.h>

#include <TaskScheduler.h>

Scheduler ts;

#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];

 float inThemperature, outThemperature;

void GetTempereature();
void OutputResult();

Task tGetTempereature(4 * TASK_SECOND, TASK_FOREVER, &GetTempereature, &ts);
Task tOutputResult(5 * TASK_SECOND, TASK_FOREVER, &OutputResult, &ts);

void GetTempereature()
{
	inThemperature = 12.459245;
	outThemperature = 19.4426032;

	_PM("Sensor ");  _PP("[");   _PP("] temp: "); _PL(inThemperature);
	_PM("Sensor ");  _PP("[");   _PP("] temp: "); _PL(outThemperature);

}

void OutputResult()
{
	snprintf(msg, MSG_BUFFER_SIZE, "%*.2f", inThemperature);
	_PM("flowmeter/0/temp/in = "); _PL(msg);
	//Serial.println(msg);
	snprintf(msg, MSG_BUFFER_SIZE, "%*.2f", outThemperature);
	_PM("flowmeter/0/temp/out = "); _PL(msg);
	//Serial.println(msg);
}

void setup()
{
	Serial.begin(74880);
	delay(100);
	_PN("Starting application...");

	tGetTempereature.enable();
	tOutputResult.enable();
}

void loop()
{
	ts.execute();
}
