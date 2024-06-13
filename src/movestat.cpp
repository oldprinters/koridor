//movestat
#include "movestat.h"

//-------------------------------------------
bool MoveStat::setMotion(bool st){
    bool res = (st == true) && (stat == false); //начало движения
    if(st){
        stat = 1;
        wait = 0;
    } else {
        wait = 1;
        setTimer();
    }
    return res;
}
//----------------------------------
bool MoveStat::cycle(){
    bool res = false;
    if(wait && getTimer()){
        wait = 0;
        stat = 0;
        res = true;
    }
    return res;
}