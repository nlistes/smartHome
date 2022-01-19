#include <cmdArduino.h>
#include <Bounce2.h>

// MC connected pins
#define PIN_OPEN 7 // to activate set value to LOW
#define PIN_OPENED 9 // if FULLY_OPENED then value is LOW (0, false)
#define PIN_CLOSE 6 // to activate set value to LOW
#define PIN_CLOSED 8 // if FULLY_CLOSED then value is LOW (0, false)

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
#define COMMAND_GOTO	140
#define COMMAND_STOP	150	//	Apturet

volatile static byte command = COMMAND_NONE;

// Action constants
#define ACTION_INIT_NONE	110
#define ACTION_INIT_INIT	111
#define ACTION_INIT_OPEN	112
#define ACTION_INIT_CLOSE	113
#define ACTION_INIT_DONE	114

volatile static byte action = ACTION_INIT_NONE;

// Direction constants
#define DIRECTION_OPEN	1 // Pieskait?t ja atveras
#define DIRECTION_CLOSE	-1

static int direction;

#define SCALE 200

volatile static unsigned long openTime = 77000, closeTime = 77000, startTime;
volatile static unsigned long position = 0, move = 0;


void setup()
{
	pinMode(PIN_OPEN, OUTPUT);
	digitalWrite(PIN_OPEN, HIGH);
	pinMode(PIN_CLOSE, OUTPUT);
	digitalWrite(PIN_CLOSE, HIGH);

	//Serial.begin(115000);
	cmd.begin(115000);
	Serial.println("Programm started...");

	pinMode(PIN_OPENED, INPUT);
	debouncer_PIN_OPENED.attach(PIN_OPENED);
	debouncer_PIN_OPENED.interval(5);

	pinMode(PIN_CLOSED, INPUT);
	debouncer_PIN_CLOSED.attach(PIN_CLOSED);
	debouncer_PIN_CLOSED.interval(5);



	//command = COMMAND_INIT;
	//action = ACTION_INIT_INIT;
	cmd.add("i", cmdInitValve);
	cmd.add("o", cmdOpenValve);
	cmd.add("c", cmdCloseValve);
	cmd.add("m", cmdMoveValve);
	cmd.add("s", cmdStopValve);
	cmd.add("do", cmdSetDirectionOpen);
	cmd.add("dc", cmdSetDirectionClose);
	//cmd.add("i", cmdInitValve);
	//cmd.add("i", cmdInitValve);
	//cmd.add("i", cmdInitValve);
	cmd.add("g", cmdGetStatuss);
}

void loop()
{
	debouncer_PIN_OPENED.update();
	debouncer_PIN_CLOSED.update();
	currentStatus = getHStatus();
	if (currentStatus != previousStatus)
	{
		Serial.print("Current status: "); Serial.println(currentStatus);
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
				Serial.println(); Serial.println("Sakta inicializacijas pozicijas ienemsana...");
				closeValve();
			}
			if (getHStatus() == STATUS_CLOSED)
			{
				action = ACTION_INIT_OPEN;
				Serial.println("Pabeigta inicializacijas pozicijas ienemsana...");
				delay(3000);
			}
			break;
		case ACTION_INIT_OPEN:
			//Serial.println("case ACTION_INIT_OPEN");
			if (getHStatus() == STATUS_CLOSED)
			{
				Serial.println(); Serial.println("Sakta inicializacijas ATVERSANA...");
				openValve();
				openTime = millis();
			}
			if (getHStatus() == STATUS_OPENED)
			{
				openTime = millis() - openTime;
				action = ACTION_INIT_CLOSE;
				Serial.println("Pabeigta inicializacijas ATVERSANA...");
				delay(3000);
			}
			break;
		case ACTION_INIT_CLOSE:
			if (getHStatus() == STATUS_OPENED)
			{
				Serial.println(); Serial.println("Sakta inicializacijas AIZVERSANA...");
				closeValve();
				closeTime = millis();
			}
			if (getHStatus() == STATUS_CLOSED)
			{
				closeTime = millis() - closeTime;
				action = ACTION_INIT_DONE;
				Serial.println("Pabeigta inicializacijas AIZVERSANA...");
				delay(3000);
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
			Serial.println(); Serial.println((getHStatus() == STATUS_OPENED) ? "Atverts!" : "Atveram...");
			openValve();
		}
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_CLOSE:
		if (getHStatus() == STATUS_UNKNOWN || getHStatus() == STATUS_OPENED)
		{
			Serial.println(); Serial.println((getHStatus() == STATUS_CLOSED) ? "Aizverts!" : "Aizveram...");
			closeValve();
		}
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_STOP:
		Serial.println(); Serial.println("STOP!!!");
		stopValve();
		command = COMMAND_NONE;
		action = ACTION_INIT_NONE;
		break;
	case COMMAND_GOTO:
		//if (getHStatus() == STATUS_CLOSED || getHStatus() == STATUS_OPENED)
		//{
		//	command = COMMAND_NONE;
		//}
		if (move < millis() - startTime)
		{
			stopValve();
			position = position + (move / round(openTime / SCALE) * direction);
		}
		break;
	default:
		break;
	}

	cmd.poll();
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
	Serial.println(); Serial.println("Inicializacija...");
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
	Serial.println(); Serial.println("Direction = OPEN");
}

void cmdSetDirectionClose(int argCnt, char** args)
{
	direction = DIRECTION_CLOSE;
	Serial.println(); Serial.println("Direction = CLOSE");
}

void cmdMoveValve(int argCnt, char** args)
{
	if (argCnt > 1)
	{
		move = round(openTime / SCALE) * cmd.conv(args[1], 10);
		Serial.print("stopTime = "); Serial.println(move);
		command = COMMAND_GOTO;
		startTime = millis();
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
	Serial.println();
	Serial.println("=== STATUSS ===");
	Serial.print("Current status: "); Serial.println(getHStatus());
	Serial.print("PIN_CLOSED: "); Serial.println(digitalRead(PIN_CLOSED));
	Serial.print("PIN_OPENED: "); Serial.println(digitalRead(PIN_OPENED));
	Serial.print("Command: "); Serial.println(command);
	Serial.print("Action: "); Serial.println(action);
	Serial.print("openTime = "); Serial.println(openTime);
	Serial.print("closeTime = "); Serial.println(closeTime);
	Serial.print("direction = "); Serial.println(direction);
	Serial.print("position = "); Serial.println(position);
	Serial.println("=== ======= ===");

}