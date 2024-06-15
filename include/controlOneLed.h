#ifndef CONTROLONELED_H
#define CONTROLONELED_H
#include<Arduino.h>
#include<Wire.h>
#include "OneLed.h"
#include "Timer.h"
#include <PubSubClient.h>

class ManagerLed: OneLed {
    Timer timerExtern;    //свет на внешние воздействие
    bool extLight{false};       //свет включен по внешней команде
    bool movement{false};   //движение
    bool presence{false};   //присутствие в зоне лидара
    bool light{false};      //светло
    bool changesMade{false};
public:
    ManagerLed(int8_t p);
	ManagerLed(int8_t p, int8_t ch);
	ManagerLed(int8_t p, int8_t ch, int8_t medium);
    void init();
    bool getStat();
    int16_t triggerAuto();
    bool setLux(float l);
    void setLidar(int16_t mm);
    void clearPr(){presence = false;}; //сброс движения
    void setMotion(bool st);
    void extLightOn();
    void setNightLevelOn();
    void setNightLevelOff();
    bool cycle();
    bool nowDay();
    // void lidarOn
};

#endif