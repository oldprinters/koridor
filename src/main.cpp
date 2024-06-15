#include <Arduino.h>
#include <WiFi.h> // library to connect to Wi-Fi network
#include "NTPClient.h"
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "Timer.h"
#include "OneLed.h"
#include <Wire.h>
#include <BH1750.h> 

#include <ld2410.h>
//-----------------------------------------------------
ld2410 radar;
bool led1_On{true};
bool led2_On{true};
int16_t dist{0};
int16_t distNew{0};
Timer tLed1(5000);
Timer tLed2(2000);

enum class Motion : uint8_t {
  None,
  Far,
  Near,
} motion;

WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 3600000);

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
const uint16_t LEVEL_LIGHT = 40;
Timer timerMqtt(1000);
//---------------------------------------------
const int16_t pinLed{12};
const int16_t MEDIUM_LEVEL = 100;
const int16_t FAR_LEVEL = 20;
const int16_t NEAR_LEVEL = 120;
OneLed oneLed(pinLed);
//--------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String str = {};
  String strTopic = topic;
  String sOut{};

  for (int i = 0; i < length; i++) {
    str += (char)payload[i];
  }

  // if(strTopic == msgHSMotion){
  //   light_1.extLightOn();
  // }
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
        // client.subscribe(msgHSMotion, 0);
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
//******************************
void setup() {
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

    // timeClient.begin();
    // timeClient.update();

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
//--------------------------------------------------------
void sendMqtt(){
  if(client.connected() && timerMqtt.getTimer())   {
    timerMqtt.setTimer();
    client.publish(msgMotion, String(dist).c_str());
  }
}
//--------------------------------------------------------
Motion radarFunc(){
	radar.read(); //Always read frames from the sensor
	if(radar.isConnected()){
		if(radar.presenceDetected()){
			if(radar.stationaryTargetDetected()){
				distNew = radar.stationaryTargetDistance();
				uint16_t energy = radar.stationaryTargetEnergy();
				// client.publish(msgMotion, String(dist).c_str());
        sendMqtt();
				if((energy > 30) && (distNew < 325)){
					dist = distNew;
					if(dist < 200){
						led1_On = true;
						tLed1.setTimer();
					}else {
						if(tLed1.getTimer()){
							led2_On = true;
							tLed2.setTimer();
						}
					}
				} 
			}
			uint16_t distMove = radar.movingTargetDistance();
			uint16_t energyMove = radar.movingTargetEnergy();
			if((distMove > 0) && (distMove < 350) && energyMove > 30){
				tLed2.setTimer();
				led2_On = true;
			}
		}
	} else {
		Serial.println(F("not connected"));
		// client.publish(msgMotion,  String(z).c_str());
	}

  if(tLed1.getTimer() && led1_On)  {
    led1_On = false;
  }
  
  if(tLed2.getTimer()  && led2_On) {
    led2_On = false;
  }
  
  if(led1_On){
		motion = Motion::Near;
  } else if(led2_On){
		motion = Motion::Far;
  } else {
		motion = Motion::None;
  }
	return motion;
}
//----------------------------------------------------------
void controlLed(Motion motion, float luxL) {
  if(luxL < LEVEL_LIGHT)
    switch (motion) {
      case Motion::None: oneLed.setOff(); break;
      case Motion::Near: oneLed.setDim(NEAR_LEVEL); break;
      case Motion::Far: oneLed.setDim(FAR_LEVEL); break;
    }
  else {
    oneLed.setOff();
  }
}
//------------------------------------------------------------------------
void loop()
{
  reconnect_mqtt();

  //....... измерение освещённости
  if (lightMeter.measurementReady()) {
    lux = lightMeter.readLightLevel();
  }
  controlLed(radarFunc(), lux);

  oneLed.cycle();
}