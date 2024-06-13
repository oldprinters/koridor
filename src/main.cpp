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
#include <BH1750.h> 

#include <ld2410.h>
//-----------------------------------------------------
ld2410 radar;
int16_t zone{0};
int16_t zoneNew{0};
int16_t dist{0};
int16_t distNew{0};
bool presenceLd{false};
Timer tLed1(5000);
Timer tLed2(2000);

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
//---------------------------------------------
const int16_t pinLed{12};
const int16_t MEDIUM_LEVEL = 100;
ManagerLed light_1(pinLed, 0, MEDIUM_LEVEL, &timeClient);
//--------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String str = {};
  String strTopic = topic;
  String sOut{};

  for (int i = 0; i < length; i++) {
    str += (char)payload[i];
  }

  if(strTopic == msgHSMotion){
    light_1.extLightOn();
  }
}
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
    pinMode(pinLedG, OUTPUT);

    Serial.begin(115200);
    Wire.begin();
    // Wire.setClock(400000); // use 400 kHz I2C
  Serial2.begin (256000, SERIAL_8N1, 16, 17); //UART for monitoring the radar
  delay(500);
  Serial.print(F("\nLD2410 radar sensor initialising: "));
  if(radar.begin(Serial2)) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("not connected"));
  }


    WiFi.begin(ssid, password); // initialise Wi-Fi and wait

    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }

    timeClient.begin();
    timeClient.update();

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
}
//------------------------------------------------------------------------
bool radarFunc(){
  radar.read(); //Always read frames from the sensor
  if(radar.isConnected()){
    if(radar.presenceDetected()){
      if(radar.stationaryTargetDetected()){
        // uint16_t dist = radar.stationaryTargetDistance();
        distNew = radar.stationaryTargetDistance();
        // Serial.println(radar.stationaryTargetEnergy());

        // zoneNew = dist / 75;
        if((radar.stationaryTargetEnergy() > 50) && (abs(distNew - dist) > 30)){
          // zone = zoneNew;
          dist = distNew;
          client.publish(msgMotion, String(distNew).c_str());
          if(distNew < 300){
            // digitalWrite(PinOutDis, LOW);
            presenceLd = true;
            if(distNew < 250){
              led1_On = true;
              tLed1.setTimer();
              digitalWrite(PinOutLd, HIGH);
            }else {
              digitalWrite(PinOutLd, LOW);
            }
            if(zoneNew > 2){
              led2_On = true;
              tLed2.setTimer();
              digitalWrite(PinOutPres, HIGH);
            }else {
              digitalWrite(PinOutPres, LOW);
            }
          } else {
            presenceLd = false;
            // digitalWrite(PinOutDis, HIGH);
            digitalWrite(PinOutPres, LOW);
            digitalWrite(PinOutLd, LOW);
          }
        } 
      }
      if(radar.movingTargetDetected()){
        uint16_t distMove = radar.movingTargetDistance();
        digitalWrite(PinOutDis, HIGH);
        tLed2.setTimer();
        led2_On = true;
      //   Serial.print(F(". Moving target: "));
      //   Serial.print(radar.movingTargetDistance());
      //   // Serial.print(F("cm energy: "));
      //   // Serial.println(radar.movingTargetEnergy());
      } else digitalWrite(PinOutDis, LOW);
      // Serial.println();
    } else {
      if(zoneNew != zone){
        client.publish(msgMotion,  String(zoneNew).c_str());
        zone = -1;
      }
      zoneNew = -1;
    }
}
//------------------------------------------------------------------------
void loop()
{
  reconnect_mqtt();

  //....... измерение освещённости
  if (lightMeter.measurementReady()) {
    lux = lightMeter.readLightLevel();
    if(light_1.setLux(lux))
      client.publish(msg_light, String(lux).c_str());
        // Serial.print("lux = ");
        // Serial.println(lux);
  }
  
  //............. движение
  light_1.cycle();
}