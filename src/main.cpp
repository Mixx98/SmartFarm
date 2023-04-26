#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

#define MotorAA_PUMP 26
#define MotorAB_PUMP 25
#define MotorAA_LED 33
#define MotorAB_LED 32
#define MOISTURE 34
#define LUX 35
#define DHTPIN 4
#define DHTTYPE    DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
sensors_event_t event;
uint8_t degreeLed; //LED 세기 (0~255)
uint8_t degreePump; //PUMP 세기 (0~255)

void setup() {
  Serial.begin(9600);
  dht.begin();
  sensor_t sensor;
  delayMS = sensor.min_delay / 1000; // 센서 버전 확인에 걸리는 시간
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);
  ledcAttachPin(MotorAA_LED, 0);
  ledcAttachPin(MotorAB_LED, 1);
  ledcAttachPin(MotorAA_PUMP, 2);
  ledcAttachPin(MotorAB_PUMP, 3);
}

void loop() {
  delay(delayMS);
  StaticJsonDocument <256> doc;
  doc["temp"] = getTemperature();
  doc["humidity"] = getHumidity();
  doc["light"] = getLux();
  doc["moisture"] = getMoisture();
  char jsondata[500];
  serializeJson(doc, jsondata);
  Serial.println(F(jsondata));
  delay(1000);  
}

// 함수선언

float getTemperature(){ //온도값 가져오기
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {    
    return event.temperature;
  }
}

float getHumidity(){ //습도값 가져오기
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {    
    return event.relative_humidity;
  }
}

unsigned short getLux(){ //조도값 가져오기
  if (isnan(analogRead(LUX))) {
    Serial.println(F("Error reading lux!"));
  }
  else {    
    return analogRead(LUX);
  }
}

unsigned short getMoisture(){ //수분량 가져오기
  if (isnan(analogRead(MOISTURE))) {
    Serial.println(F("Error reading moisture!"));
  }
  else {    
    return analogRead(MOISTURE);
  }
}

void setLed(){ //LED 작동
  ledcWrite(0, 0);
  ledcWrite(1, degreeLed);
}

void setPump(){ //펌프 작동
  ledcWrite(2, 0);
  ledcWrite(3, degreePump);
}