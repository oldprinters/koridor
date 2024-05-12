#include "managerLed.h"


//------------------------------------------------
void ManagerLed::init(){
    moveStat = new MoveStat();
    moveStat -> setDt(FOLLOW_ME_TIME);
    queue.push_back(Events::MOVE);
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
    changesMade = false;

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
    queue.push_back(Events::BUTTOM);
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
    //presence
    bool pr = mm < MAX_LENGTH? true: false;
    if(pr != presence){
        queue.push_back(Events::LIDAR);
        presence = pr;
    }
}
//---------------------------------------------
void ManagerLed::setMotion(bool st){
    if(moveStat->setMotion(st))
        queue.push_back(Events::MOVE); //фиксируем движение
}
//-------------------------------------
bool ManagerLed::cycle(){
    OneLed::cycle();    //текущее состояние
    if(moveStat->cycle())
        queue.push_back(Events::MOVE); //фиксируем движение
    bool res = moveStat->getStat();   //наличие движения
    if( extLight && timerExtern.getTimer() ){
        setNightLevelOff();
        extLight = false;
    }
    if(queue.size() > 0){
        for(auto d: queue){
            switch(d){
                case Events::BUTTOM: break;
                case Events::LIDAR: 
                    if(presence && (stat == Status::AUTO)){
                        if(!light){
                            OneLed::setStat(StatLed::ON);
                            OneLed::setMediumLevel();   
                            extLight = false;
                        } else {
                            OneLed::setOff();
                        }
                    }
                    break;
                case Events::MOVE:
                    if( res ){
                        if(stat == Status::AUTO && !OneLed::getStatOn() && !light){
                            OneLed::setStat(StatLed::ON);
                            extLight = false;
                            // if(nowDay())
                            OneLed::setMediumLevel();
                            // else
                            //     OneLed::setNightLevel();
                        }
                    } else {
                        pSensor->read();
                        // Serial.print((*pSensor).ranging_data.range_status != VL53L1X::RangeStatus::RangeValid);
                        // Serial.print(", mm = ");
                        // Serial.println((*pSensor).ranging_data.range_mm);
                        if(((*pSensor).ranging_data.range_status != VL53L1X::RangeStatus::RangeValid)
                            && ((*pSensor).ranging_data.range_mm > MAX_LENGTH)){
                                if(stat == Status::AUTO){
                                    OneLed::setOff();
                                }
                        }
                    }
                    break;
                case Events::LIGHT: if(stat == Status::AUTO && !OneLed::getStatOn() && !light && 
                ((*pSensor).ranging_data.range_status == VL53L1X::RangeStatus::RangeValid) &&
                ((*pSensor).ranging_data.range_mm < MAX_LENGTH)){
                        OneLed::setStat(StatLed::ON);
                        OneLed::setMediumLevel();
                        extLight = false;
                    } else if(stat == Status::AUTO && (OneLed::getStat() == StatLed::ON) && light){
                        OneLed::setStat(StatLed::OFF);
                    }
                    break;
            }
        }
        queue.clear();
        changesMade = true;
    }
    return res;
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
        queue.push_back(Events::LIGHT);
        res = true;
    }
    lux = l; 
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
//------------------------------------------------
void  ManagerLed::printQueue(){
    if(queue.size() > 0){
        Serial.print("queue: ");
        for(auto i: queue){
            Serial.println(static_cast<int>(i));
        }
    }
}