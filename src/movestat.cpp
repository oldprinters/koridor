//movestat
#include "movestat.h"

//-------------------------------------------
bool MoveStat::setMotion(bool st){
    bool res = (st == true) && (stat == false);
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
    if(wait && getTimer()){
        wait = 0;
        stat = 0;
        return true;
    }
    return false;
}