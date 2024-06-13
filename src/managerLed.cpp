#include "managerLed.h"


//------------------------------------------------
void ManagerLed::init(){
    moveStat = new MoveStat();
    moveStat -> setDt(FOLLOW_ME_TIME);
    timerExtern.setDt(5000);
}
//_____-------------------------------------------
ManagerLed::ManagerLed(int8_t p, NTPClient *pNtp):OneLed(p), pNtpClient(pNtp){
    this->init();
}
ManagerLed::ManagerLed(int8_t p, int8_t ch, NTPClient *pNtp): OneLed(p, ch), pNtpClient(pNtp){
    this->init();
}
ManagerLed::ManagerLed(int8_t p, int8_t ch, int8_t medium, NTPClient *pNtp): OneLed(p, ch, medium), pNtpClient(pNtp){
    this->init();
}
//------------------------------------------------
void ManagerLed::linkLidar(VL53L1X* pS){
    if(pS != nullptr)
        pSensor = pS;
}
//-------------------------------------------
char const* ManagerLed::getModeName(){
    char const* pStr{nullptr};
    switch(stat){
        case Status::ON: pStr = "On"; break;
        case Status::OFF: pStr = "Off"; break;
        case Status::AUTO: pStr = "Auto"; break;
        default: pStr = "Def"; break;
    }
    return pStr;
}
//-------------------------------------------
int16_t ManagerLed::triggerAuto(){
    // Serial.println(static_cast<int16_t>(stat));
    switch(stat){
        case Status::ON: 
        case Status::OFF: stat = Status::AUTO;
            OneLed::setStat(StatLed::ON);
            OneLed::setMediumLevel();
            break;
        case Status::AUTO: stat = Status::OFF;
            OneLed::setOff();
            break;
    }
    return static_cast<int16_t>(stat);
}
//-------------------------------------------
int16_t ManagerLed::clickBut(int16_t nBut, bool shortClick, int16_t nClick){
    if(nBut == 0){
        if(!shortClick){
            toggleMax();
            stat = Status::ON;
        } else {
            switch(nClick){
                case 1: triggerAuto();break;
                case 2: stat = Status::ON; OneLed::setMediumLevel(); break;
                case 3: stat = Status::ON; OneLed::setNightLevel(); break;
            }
        }
    } else {
        //пока мыслей нет
    }
    return 1;//this->getStat() == 1;
}
//-------------------------------------------------------
void ManagerLed::extLightOn(){
    if(!light && (OneLed::getStat() == StatLed::OFF)){
        setNightLevelOn();
        timerExtern.setTimer();
        extLight = true;
    }
}
//-------------------------------------------------------
void ManagerLed::setNightLevelOn(){
    if(stat == Status::AUTO){
        // stat = Status::ON; 
        OneLed::setNightLevel();
    }
}
//-------------------------------------------------------
void ManagerLed::setNightLevelOff(){
    // stat = Status::AUTO;
    OneLed::setOff();
}
//-------------------------------------------------------
//если появился объект, то в режиме AUTO включается свет
void ManagerLed::setLidar(int16_t mm){
    presence = mm < MAX_LENGTH? true: false; //присутствие
}
//---------------------------------------------
void ManagerLed::setMotion(bool st){
    moveStat->setMotion(st);
}
//-------------------------------------
bool ManagerLed::cycle(){
    OneLed::cycle();    //текущее состояние
    moveStat->cycle();

    bool moveOn = moveStat->getStat();   //наличие движения

    if( extLight && timerExtern.getTimer() ){
        setNightLevelOff();
        extLight = false;
    }

    if(stat == Status::AUTO){
        if(moveOn || presence){
            if(!light && !OneLed::getStatOn()){
                    OneLed::setStat(StatLed::ON);
                    OneLed::setMediumLevel();
                    extLight = false;
            }
        } else {
            if(OneLed::getStat() == StatLed::ON)
                OneLed::setStat(StatLed::OFF);
        }
    }

    return true;
}
//----------------------------------------------------
bool ManagerLed::getStat(){
    bool res = (stat == Status::ON) 
        || ((stat == Status::AUTO) && (OneLed::getStat() == StatLed::ON));
    return res;
}
//--------------------------------------------------------
bool ManagerLed::setLux(float l){
    int dlight = light;
    bool res = false;
    if(l < LEVEL_LIGHT - D_LEVEL_LIGHT ) {
        light = false;
    } else if( l > LEVEL_LIGHT + D_LEVEL_LIGHT) {
        light = true;
    }
    if(dlight != light){
        res = true;
    }
    // Serial.print("light = ");
    // Serial.println(light);
    return res;
}
//------------------------------------------------------------------------
bool ManagerLed::nowDay(){
//   bool res = false;
//   int day = pNtpClient->getDay();
//   int hour = pNtpClient->getHours();
//   if( (day >= 0) && (day < 5) )
//     res = (hour > 5) && (hour < 23);
//   else
//     res = (hour > 8) && (hour < 24);
//   return res;
    return true;
}
