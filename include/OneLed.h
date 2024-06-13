// OneLed.h

#ifndef _ONELED_h
#define _ONELED_h
#include "Timer.h"

//#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
// #else
// 	#include "WProgram.h"
// #endif
#define TIMER_T 50
enum class StatLed { OFF, ON };
enum class StatPower { OFF, ON, MOVE, PAUSE_OFF, DIST };

class OneLed: public Timer
{
	const int pin;
	int level { 0 };
	int levelDim { 50 };
	int maxLevel {100};
	int lowLevel {5};
	int mediumLevel {50};
	int nightLevel {2};
	StatLed statLed { StatLed::ON };
	int ledChannel = 0;
	const int resolution = 8;
	const int freq = 100;
public:
	OneLed(int p);
	OneLed(int p, int ch);
	OneLed(int p, int ch, int medium);
	OneLed(int p, int ch, int medium, int nLevel);
	int getPin() { return pin; }
	int getLevelDim(){return levelDim;}
	StatLed getStat(){return statLed;}
	bool getStatOn();
	void setStat(StatLed stat);
	bool trigger();	//изменение состояния на обратное
	void toggleMax();
	bool onMaxLevel();
	void setLowLevel(){maxLevel = lowLevel; levelDim = lowLevel;}
	void setMediumLevel(){maxLevel = mediumLevel; levelDim = mediumLevel;}
	void setNightLevel(){maxLevel = nightLevel; levelDim = nightLevel;}
	void setOff();
	void setDim(int l);
	void setMaxLevel(int i);
	int getMaxLevel(){return maxLevel;}
	virtual bool cycle();
	int arDt[6]={ 1, 1, 2, 3, 5, 10};
};

#endif

