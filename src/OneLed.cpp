#include "OneLed.h"

//***********************************************
	OneLed::OneLed(int p): Timer(TIMER_T), pin(p) { 
		pinMode(pin, OUTPUT); 
		ledcSetup(ledChannel, freq, resolution);
		ledcAttachPin(pin, ledChannel);
	}
//***********************************************
	OneLed::OneLed(int p, int ch = 0): Timer(TIMER_T), pin(p), ledChannel(ch) { 
		pinMode(pin, OUTPUT); 
		ledcSetup(ledChannel, freq, resolution);
		ledcAttachPin(pin, ledChannel);
	}
//***********************************************
	OneLed::OneLed(int p, int ch, int medium): Timer(TIMER_T), pin(p), ledChannel(ch), mediumLevel(medium) { 
		pinMode(pin, OUTPUT); 
		ledcSetup(ledChannel, freq, resolution);
		ledcAttachPin(pin, ledChannel);
	}
//***********************************************
	OneLed::OneLed(int p, int ch, int medium, int nLevel): Timer(TIMER_T), pin(p), ledChannel(ch), mediumLevel(medium), nightLevel{nLevel} { 
		pinMode(pin, OUTPUT); 
		ledcSetup(ledChannel, freq, resolution);
		ledcAttachPin(pin, ledChannel);
	}
//*************************************************
void OneLed::setDim(int l) {
	levelDim = l;
}
//***********************************************
bool OneLed::onMaxLevel(){
	if(statLed == StatLed::OFF) {
		maxLevel  = 255;
		levelDim = maxLevel;
		statLed = StatLed::ON;
		return true;
	} else return false;
}
//***********************************************
void OneLed::toggleMax(){
	levelDim = (levelDim > 250? mediumLevel: 255);
	// levelDim = mediumLevel;
	// Serial.print("maxLevel = ");                                                                                                                                                                                                                                                                                                             
	// Serial.print(maxLevel);
	// Serial.print(", levelDim = ");
	// Serial.print(levelDim);
	// Serial.print(", mediumLevel = ");
	// Serial.println(mediumLevel);
	statLed == StatLed::ON;
}
//-------------------------------------------------------
bool OneLed::getStatOn(){
    return (statLed == StatLed::ON) && (levelDim > nightLevel);
}
//***********************************************
bool OneLed::trigger(){
	switch (statLed) {
		case StatLed::ON:levelDim = 0; statLed = StatLed::OFF; break;
		default:levelDim = mediumLevel; statLed = StatLed::ON; break;
	}
	return statLed == StatLed::ON;
}
//*********************************************
void OneLed::setMaxLevel(int i){
	maxLevel = i;
	if(statLed == StatLed::ON) {
		levelDim = maxLevel;
	}
}
//*********************************************
void OneLed::setOff(){
	statLed = StatLed::OFF;
	maxLevel = mediumLevel;
	levelDim = 0;
}
//*********************************************
void OneLed::setStat(StatLed stat) {
	statLed = stat; 
	switch (statLed) {
		case StatLed::OFF:levelDim = 0; break;
		case StatLed::ON:levelDim = maxLevel; break;
	}
}
//*************************************
bool OneLed::cycle() {
	if (level != levelDim) {
		if (getTimer()) {
			setTimer();
			int dt = arDt[int(std::floor(level / 50))];
			if(level - dt < 0)dt = 1;
			if((level < levelDim) && (level + dt > levelDim))
				dt = levelDim - level;
			level += (level < levelDim? dt: -1 * dt);
			ledcWrite(ledChannel, level); //14.05.2024 ptr
		}
		return true;
	} else {
		return false;
	}
}
//======================================

