#ifndef MANAGERLED_H
#define MANAGERLED_H
#include "OneLed.h"
#include "Timer.h"
#include "movestat.h"
#include "NTPClient.h"
#include <VL53L1X.h>
#include <Arduino.h>
#include <vector>

const int16_t LEVEL_LIGHT = 120;
const int16_t D_LEVEL_LIGHT = 10;

enum class Status { OFF, ON, AUTO };
enum class Events {  NOT_E, BUTTOM, LIDAR, MOVE, LIGHT, EXTERN };

class ManagerLed: OneLed {
    Status stat{Status::AUTO};
    Timer timerExtern;    //свет на внешние воздействие
    bool extLight{false};       //свет включен по внешней команде
    int16_t lux{200};         //освещённость
    bool movement{false};   //движение
    bool presence{false};   //присутствие в зоне лидара
    bool light{false};      //светло
    bool changesMade{false};
    MoveStat* moveStat;
    VL53L1X* pSensor{nullptr};       //указатель на lidar
    std::vector<Events> queue;
    NTPClient *pNtpClient{nullptr};
public:
    ManagerLed(int8_t p, NTPClient *pNtp);
	ManagerLed(int8_t p, int8_t ch, NTPClient *pNtp);
	ManagerLed(int8_t p, int8_t ch, int8_t medium, NTPClient *pNtp);
    ~ManagerLed(){delete moveStat;}
    void init();
    bool getChangesMade(){return changesMade;}
    int8_t getQueueSize(){return queue.size();}
    const char* getModeName();
    void printQueue();
    void linkLidar(VL53L1X* pS);
    int16_t clickBut(int16_t nBut, bool longClick, int16_t nClick); //обработка события click
    bool getStat();
    int16_t triggerAuto();
    void setLux(float l);
    void setLidar(int16_t mm);
    void setMotion(bool st);
    void extLightOn();
    void setNightLevelOn();
    void setNightLevelOff();
    bool cycle();
    bool nowDay();
    // void lidarOn
};

#endif // MANAGERLED_H