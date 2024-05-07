#ifndef MOVESTAT_H
#define MOVESTAT_H
#include "Timer.h"
const int16_t FOLLOW_ME_TIME = 10000;
const int16_t MAX_LENGTH {1900};

class MoveStat:Timer {
    bool stat{1};
    bool wait{0};   //ожидание перед выключением
  public:
    MoveStat():Timer(FOLLOW_ME_TIME){};
    MoveStat(uint32_t dt):Timer(dt){};
    bool getStat(){return stat;}
    bool setMotion(bool st);
    bool cycle();
    void setDt(uint32_t dt) {Timer::setDt(dt);}
};

#endif