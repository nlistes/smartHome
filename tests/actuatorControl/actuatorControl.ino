#include <Bounce2.h>
#include <Cmd.h>

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

// MC connected pins (ATMEGA328P)
//#define PIN_OPEN	7 // ESP=D3 (pull-up) To activate set value to LOW
//#define PIN_OPENED	9 // ESP=D1 (integrated pull-up) If FULLY_OPENED then value is LOW (0, false)
//#define PIN_CLOSE	6 // ESP=D4 (pull-up) To activate set value to LOW
//#define PIN_CLOSED	8 // ESP=D2 (integrated pull-up) If FULLY_CLOSED then value is LOW (0, false)

// MC connected pins (ESP8266)
#define PIN_OPEN	D3 // to activate set value to LOW
#define PIN_OPENED	D1 // if FULLY_OPENED then value is LOW (0, false)
#define PIN_CLOSE	D4 // to activate set value to LOW
#define PIN_CLOSED	D2 // if FULLY_CLOSED then value is LOW (0, false)

Bounce debouncer_PIN_OPENED = Bounce();
Bounce debouncer_PIN_CLOSED = Bounce();

// Status constants
#define STATUS_UNKNOWN	1	//	Gatavs darbibai, stav ne gala poziicijas
#define STATUS_OPENED	2	//	Aizverts, motors izslegts
#define STATUS_CLOSED	3	//	Atverts, motors izslegts
#define STATUS_OPENING	4	//	Aizveras, motors ieslegts
#define STATUS_CLOSING	5	//	Atveras, motors ieslegts
#define STATUS_INIT		6

volatile static byte status, currentStatus, previousStatus;

// Command constants
#define COMMAND_NONE	100
#define COMMAND_INIT	110
#define COMMAND_OPEN	120	//	Aizvert
#define COMMAND_CLOSE	130	//	Atvert
#define COMMAND_MOVE	140
#define COMMAND_POS		150
#define COMMAND_STOP	160	//	Apturet

volatile static byte command = COMMAND_NONE;

// Action constants
#define ACTION_INIT_NONE	110
#define ACTION_INIT_INIT	111
#define ACTION_INIT_OPEN	112
#define ACTION_INIT_CLOSE	113
#define ACTION_INIT_DONE	114

volatile static byte action = ACTION_INIT_NONE;

// Direction constants
#define DIRECTION_OPEN	1 // Pieskaitit ja atveras
#define DIRECTION_CLOSE	-1

static int direction = 0;

#define SCALE 100

volatile static unsigned long openTime = 77000, closeTime = 77000, movementUnit = 0;
volatile static unsigned long moveTimePlanned = 0, movementStartTime = 0, moveTimeReal = 0;

byte actualPosition = 0;


void setup()
{
	pinMode(PIN_OPEN, OUTPUT);
	digitalWrite(PIN_OPEN, HIGH);
	pinMode(PIN_CLOSE, OUTPUT);
	digitalWrite(PIN_CLOSE, HIGH);

	Serial.begin(74880);
	delay(100);
	cmdInit(&Serial);
	_PL("Programm started...");

	pinMode(PIN_OPENED, INPUT);
	debouncer_PIN_OPENED.attach(PIN_OPENED);
	debouncer_PIN_OPENED.interval(5);

	pinMode(PIN_CLOSED, INPUT);
	debouncer_PIN_CLOSED.attach(PIN_CLOSED);
	debouncer_PIN_CLOSED.interval(5);

	//command = COMMAND_INIT;
	//action = ACTION_INIT_INIT;

	cmdAdd("i", cmdInitValve);
	cmdAdd("o", cmdOpenValve);
	cmdAdd("c", cmdCloseValve);
	cmdAdd("m", cmdMoveValve);
	cmdAdd("s", cmdStopValve);
	cmdAdd("do", cmdSetDirectionOpen);
	cmdAdd("dc", cmdSetDirectionClose);
	cmdAdd("g", cmdGetStatuss);

}

void loop()
{
	debouncer_PIN_OPENED.update();
	debouncer_PIN_CLOSED.update();
	currentStatus = getHStatus();
	if (currentStatus != previousStatus)
	{
		_PP("Current status: "); _PL(currentStatus);
		previousStatus = currentStatus;
	}
	//if (debouncer_PIN_OPENED.changed())
	switch (command)
	{
	case COMMAND_INIT:
		switch (action)
		{
		case ACTION_INIT_INIT:
			if (getHStatus() == STATUS_UNKNOWN || getHStatus() == STATUS_OPENED)
			{
				_PL(); _PL("Sakta inicializacijas pozicijas ienemsana...");
				closeValve();
			}
			if (getHStatus() == STATUS_CLOSED)
			{
				action = ACTION_INIT_OPEN;
				_PL("Pabeigta inicializacijas pozicijas ienemsana...");
				delay(2000);
			}
			break;
		case ACTION_INIT_OPEN:
			//_PL("case ACTION_INIT_OPEN");
			if (getHStatus() == STATUS_CLOSED)
			{
				_PL(); _PL("Sakta inicializacijas ATVERSANA...");
				openValve();
				openTime = millis();
			}
			if (getHStatus() == STATUS_OPENED)
			{
				openTime = millis() - openTime;
				movementUnit = openTime / SCALE;
				action = ACTION_INIT_CLOSE;
				_PL("Pabeigta inicializacijas ATVERSANA...");
				_PP("openTime = "); _PL(openTime);
				delay(2000);
			}
			break;
		case ACTION_INIT_CLOSE:
			if (getHStatus() == STATUS_OPENED)
			{
				_PL(); _PL("Sakta inicializacijas AIZVERSANA...");
				closeValve();
				closeTime = millis();
			}
			if (getHStatus() == STATUS_CLOSED)
			{
				closeTime = millis() - closeTime;
				action = ACTION_INIT_DONE;
				_PL("Pabeigta inicializacijas AIZVERSANA...");
				_PP("closeTime = "); _PL(closeTime);
				delay(2000);
			}
			break;
		case ACTION_INIT_DONE:
			command = COMMAND_NONE;
			break;
		default:
			break;
		}
		break;
	case COMMAND_OPEN:
		if (getHStatus() == STATUS_UNKNOWN || getHStatus() == STATUS_CLOSED)
		{
			_PL(); _PL((getHStatus() == STATUS_OPENED) ? "Atverts!" : "Atveram...");
			openValve();
		}
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_CLOSE:
		if (getHStatus() == STATUS_UNKNOWN || getHStatus() == STATUS_OPENED)
		{
			_PL(); _PL((getHStatus() == STATUS_CLOSED) ? "Aizverts!" : "Aizveram...");
			closeValve();
		}
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_STOP:
		_PL(); _PL("STOP!!!");
		stopValve();
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_MOVE:
		//if (getHStatus() == STATUS_CLOSED || getHStatus() == STATUS_OPENED)
		//{
		//	command = COMMAND_NONE;
		//}
		moveTimeReal = millis() - movementStartTime;
		if (moveTimePlanned < moveTimeReal)
		{
			stopValve();
			_PL(); _PP("moveTimeReal = "); _PL(moveTimeReal);
			actualPosition = actualPosition + (moveTimeReal / SCALE) * direction;
			command = COMMAND_NONE;
		}
		break;
	default:
		break;
	}

	cmdPoll();
}

int getHStatus()
{
	int result = STATUS_UNKNOWN;
	if (digitalRead(PIN_OPEN) == LOW)	result = STATUS_OPENING;
	if (digitalRead(PIN_CLOSE) == LOW)	result = STATUS_CLOSING;
	if (digitalRead(PIN_OPEN) == LOW && debouncer_PIN_OPENED.read() == LOW)	result = STATUS_OPENED; // is FULLY_OPENED
	if (digitalRead(PIN_CLOSE) == LOW && debouncer_PIN_CLOSED.read() == LOW)	result = STATUS_CLOSED; // is FULLY_CLOSED
	return result;

}

void openValve()
{
	{
		digitalWrite(PIN_CLOSE, HIGH);
		digitalWrite(PIN_OPEN, LOW);
	}
}

void closeValve()
{
	{
		digitalWrite(PIN_OPEN, HIGH);
		digitalWrite(PIN_CLOSE, LOW);
	}
}

void stopValve()
{
	digitalWrite(PIN_OPEN, HIGH);
	digitalWrite(PIN_CLOSE, HIGH);
}

void cmdInitValve(int argCnt, char** args)
{
	_PL(); _PL("Inicializacija...");
	command = COMMAND_INIT;
	action = ACTION_INIT_INIT;
}

void cmdOpenValve(int argCnt, char** args)
{
	command = COMMAND_OPEN;
}

void cmdCloseValve(int argCnt, char** args)
{
	command = COMMAND_CLOSE;
}

void cmdSetDirectionOpen(int argCnt, char** args)
{
	direction = DIRECTION_OPEN;
	_PL(); _PL("Direction = OPEN");
}

void cmdSetDirectionClose(int argCnt, char** args)
{
	direction = DIRECTION_CLOSE;
	_PL(); _PL("Direction = CLOSE");
}

void cmdMoveValve(int argCnt, char** args)
{
	if (argCnt > 1)
	{
		moveTimePlanned = movementUnit * cmdStr2Num(args[1], 10);
		_PP("args = "); _PL(cmdStr2Num(args[1], 10));
		_PP("moveTimePlanned = "); _PL(moveTimePlanned);
		command = COMMAND_MOVE;
		movementStartTime = millis();
		if (direction == DIRECTION_OPEN)
		{
			openValve();
		}
		if (direction == DIRECTION_CLOSE)
		{
			closeValve();
		}

	}
}

void cmdStopValve(int argCnt, char** args)
{
	command = COMMAND_STOP;
}

void cmdGetStatuss(int argCnt, char** args)
{
	_PL();
	_PL("====== STATUSS ======");
	_PP("Current status: "); _PL(getHStatus());
	_PP("PIN_CLOSED: "); _PL(digitalRead(PIN_CLOSED));
	_PP("PIN_OPENED: "); _PL(digitalRead(PIN_OPENED));
	_PP("Command: "); _PL(command);
	_PP("Action: "); _PL(action);
	_PP("openTime = "); _PL(openTime);
	_PP("closeTime = "); _PL(closeTime);
	_PP("movementUnit = "); _PL(movementUnit);
	_PP("direction = "); _PL(direction);
	_PP("actualPosition = "); _PL(actualPosition);
	_PL("====== ======= ======");

}