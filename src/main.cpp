#include <Arduino.h>
#include <WiFi.h> // library to connect to Wi-Fi network
#include "NTPClient.h"
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "Timer.h"
#include "movestat.h"
#include "managerLed.h"
#include "OneLed.h"
#include <Wire.h>
#include <VL53L1X.h>
#include <BH1750.h> 

VL53L1X sensor;
Timer* timer53L1;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 3600000);
NTPClient* ptk = &timeClient;

const int16_t pinMove{14};
volatile bool motion{0};  //признак изменения движения
// //*--------------------------------------------
const int pinLedR{27};
const int pinLedG{26};
const int pinLedB{25};
//---------------------------------------------
const char* ssid = "ivanych";
const char* password = "stroykomitet";
WiFiClient espClient;
PubSubClient client(espClient);
//---------------------------------------------
const char* msg_lidar = "koridor/lidar";
const char* msg_light = "koridor/light";
const char* mqtt_server = "192.168.1.34";
const char* msgMotion = "aisle/motion";
const char* msgHSMotion = "hall_small/motion";
const char* msgLightOff = "koridor/lightOff";
const char* msgLightOn = "koridor/lightOn";
const char* lightNightOn = "koridor/lightNightOn";
const char* lightNightOff = "koridor/lightNightOff";
//---------------------------------------------
BH1750 lightMeter(0x23);  //0x5c 23
float lux{8000};  //яркость света в помещении
//--------------------------------------------- кнопки
const int16_t pinBut1{16};
volatile int16_t buttonStatus_1{0}, ft_1{0}, ft_1_dr{0}, nClickBut_1{0};
const int drDelay{250};
const int pauseDelay{2000};
int16_t ft_1_pause{0};
Timer tLux(500);
Timer tButt_1(1000);
Timer tDrebezg_1(drDelay);
Timer tPause_1(pauseDelay);
//---------------------------------------------
const int16_t pinLed{12};
const int16_t MEDIUM_LEVEL = 100;
ManagerLed light_1(pinLed, 0, MEDIUM_LEVEL, &timeClient);
//*********************************************
void IRAM_ATTR button_interr_1(){ //IRAM_ATTR
    detachInterrupt(pinBut1);
  ft_1_dr = 2;
  if(!ft_1){
    ft_1 = 3;
    nClickBut_1 = 1;
  } else if(ft_1 == 2){
    nClickBut_1++;
  }
}
//--------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String str = {};
  String strTopic = topic;
  String sOut{};

  for (int i = 0; i < length; i++) {
    str += (char)payload[i];
  }

  if(strTopic == msgMotion){
  } else if(strTopic == msgHSMotion){
    light_1.extLightOn();
  } else if(strTopic == msgLightOff){
  } else if(strTopic == msgLightOn){
  } else if(strTopic == lightNightOn){
  } else if(strTopic == lightNightOff){
  }
}
//-----------------------------------
//-----------------------------------
void reconnect_mqtt() {
  // Loop until we're reconnected
  if(!client.connected()) {
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");
      String clientId = "Koridor-Ptr";
      // Attempt to connect
      Serial.println(clientId);
      if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        client.subscribe(msgMotion, 0);
        client.subscribe(msgHSMotion, 0);
        client.subscribe(msgLightOff, 0);
        client.subscribe(msgLightOn, 0);
      } else {
        Serial.print("failed, rc= ");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  } else {
    client.loop();  //mqtt
  }
}
//*********************************************
void IRAM_ATTR moving(){
  motion = true;
}
//******************************
void setup()
{
    pinMode(pinMove, INPUT);
    // pinMode(pinLedR, OUTPUT);
    pinMode(pinLedG, OUTPUT);
    // pinMode(pinLedB, OUTPUT);

    tLux.setTimer();
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000); // use 400 kHz I2C

    sensor.setTimeout(500);
    if (!sensor.init())
    {
      Serial.println("Failed to detect and initialize sensor!");
      while (1);
    }

    // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
    // You can change these settings to adjust the performance of the sensor, but
    // the minimum timing budget is 20 ms for short distance mode and 33 ms for
    // medium and long distance modes. See the VL53L1X datasheet for more
    // information on range and timing limits.
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.setROISize(16, 16);

    // Start continuous readings at a rate of one measurement every 50 ms (the
    // inter-measurement period). This period should be at least as long as the
    // timing budget.
    sensor.startContinuous(50);

    light_1.linkLidar(&sensor);

    WiFi.begin(ssid, password); // initialise Wi-Fi and wait
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }
    timeClient.begin();
    timeClient.update();
    timer53L1 = new Timer(500);
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    reconnect_mqtt();
    //---------------------------
    if (lightMeter.begin()) {//BH1750::CONTINUOUS_HIGH_RES_MODE)) {
      Serial.println(F("BH1750 Advanced begin"));
    }
    else {
      Serial.println(F("Error initialising BH1750"));
    }

    attachInterrupt(digitalPinToInterrupt(pinMove), moving, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinBut1), button_interr_1, FALLING);
}
//------------------------------------------------------------------------
int16_t motion_func(){
  bool res{false};
  if(motion){
    res = true;
    motion = false;
    int p = digitalRead(pinMove);
    light_1.setMotion(p);
    if( p )client.publish(msgMotion, "1");
    else client.publish(msgMotion, "0");
  }
  return res;
}
//------------------------------------------------------------------------
void loop()
{
  reconnect_mqtt();

  if(timer53L1->getTimer()){  //дальномер
    sensor.read();
    timer53L1->setTimer();
    if(sensor.ranging_data.range_status == VL53L1X::RangeStatus::RangeValid){
        light_1.setLidar(sensor.ranging_data.range_mm);
        client.publish(msg_lidar, String(sensor.ranging_data.range_mm).c_str());
    }
  }
  //....... измерение освещённости
  if (tLux.getTimer() && lightMeter.measurementReady()) {
    tLux.setTimer();
    lux = lightMeter.readLightLevel();
    light_1.setLux(lux);
    client.publish(msg_light, String(lux).c_str());
        // Serial.print("lux = ");
        // Serial.println(lux);
  }
  
  motion_func();  //проверка движения

  // //-------------------------------------------------------------------------------- BUTTONS
  // if(ft_1 == 3){
  //   tButt_1.setTimer();
  //   ft_1 = 2;
  // }
  // if( ft_1_dr == 2 ){
  //   tDrebezg_1.setTimer();
  //   ft_1_dr = 1;
  // }
  // if( ( ft_1_dr == 1 ) && tDrebezg_1.getTimer() ){
  //   ft_1_dr = 0;
  //   attachInterrupt(digitalPinToInterrupt(pinBut1), button_interr_1, FALLING);
  // }
  // if( ( ft_1_pause == 1 ) && tPause_1.getTimer() ){
  //   ft_1_pause = 0;
  //   attachInterrupt(digitalPinToInterrupt(pinBut1), button_interr_1, FALLING);
  // }

  // if(ft_1 == 2 && tButt_1.getTimer()){
  //   int16_t i = digitalRead(pinBut1);
  //   // Serial.print(i == 0? "Long ": "Short ");
  //   // Serial.print("nCount = ");
  //   // Serial.println(nClickBut_1);
  //   ft_1 = 0;
  //   ft_1_pause = 1;
  //   tPause_1.setTimer();
  //   detachInterrupt(pinBut1);
  //   light_1.clickBut(0, i, nClickBut_1);
  // }


  light_1.cycle();
}