#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define MotorAA_PUMP 26
#define MotorAB_PUMP 25
#define MotorAA_LED 33
#define MotorAB_LED 32
#define MOISTURE 34
#define LUX 35
#define DHTPIN 4
#define DHTTYPE DHT11
#define gmtOffset_sec 32400
#define daylightOffset_sec 0

WiFiClient espClient;
PubSubClient client(espClient);
TaskHandle_t Task1;
TaskHandle_t Task2;
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;

const char *ssid = "803";
const char *password = "y55067725";
const char *ntpServer = "kr.pool.ntp.org";
const char *mqtt_broker = "3.34.199.220";
const char *ID = "unit001";
const char *pubTopic = "data/unit001";
const char *subTopic = "cmd/unit001/#";
const char *mqtt_username = "yoon";
const char *mqtt_password = "helloElice";


/*######################################*/
// 함수선언

void reconnect() { //MQTT연결
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect(ID,mqtt_username,mqtt_password)) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
   
      client.subscribe(subTopic);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(F(client.state()));
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setLed(uint8_t degreeLed){ //LED 작동
  ledcWrite(0, 0);
  ledcWrite(1, degreeLed);
}

void setPump(uint8_t degreePump){ //펌프 작동
  ledcWrite(0, 0);
  ledcWrite(2, degreePump);
}

void setActuator(char* topic, uint8_t value){
  /*
  topic : [cmd/unit001/액츄에이터이름]
  message : 0~255
  */
  
  //액츄에이터 이름 추출
  char* pump = strstr(topic, "pump");
  char* led = strstr(topic, "led");

  //서버에서 LED,PUMP 동작 명령을 받았을때
  if(pump != NULL){
    setPump(value);
  }
  else if (led != NULL)  {
    setLed(value);
  }  
}

void callBack(char* topic, byte* payload, unsigned int length) {
  char receiveMessage[5];  

  Serial.print(F("Message arrived ["));
  Serial.print(F(topic));
  Serial.print(F("] : "));
  for (int i = 0; i < length; i++) {
    receiveMessage[i]=(char)payload[i];
  }
  receiveMessage[length] = '\0'; // null 문자 추가

  Serial.println(F(receiveMessage)); 

  uint8_t value = (uint8_t) atoi(receiveMessage); // 문자열을 정수형으로 변환
  setActuator(topic, value);
}

float getTemperature(){ //온도값 가져오기
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    return 0;
  }
  else {    
    return event.temperature;
  }
}

uint8_t getHumidity(){ //습도값 가져오기
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    return 0;
  }
  else {    
    return event.relative_humidity;
  }
}

uint16_t getLux(){ //조도값 가져오기
  uint16_t luxValue = analogRead(LUX);
  if (isnan(luxValue)) {
    Serial.println(F("Error reading lux!"));
    return 0;
  }
  else {    
    return luxValue;
  }
}

uint16_t getMoisture(){ //수분량 가져오기
  uint16_t moistureValue = analogRead(MOISTURE);
  if (isnan(moistureValue)) {
    Serial.println(F("Error reading moisture!"));
    return 0;
  }
  else {    
    return 4095-moistureValue;
  }
}

// 듀얼 코어 관리 부분

//0번 코어
void send(void *param){  
  StaticJsonDocument <256> doc; //https://arduinojson.org/v6/api/jsondocument/ JSON API

  while(1){  
    //JSON구조체에 센서값 4개 입력  
    doc["device_id"] = ID;  
    doc["temp"] = roundf(getTemperature()*100.00)/100.00;
    doc["humidity"] = getHumidity();
    doc["light"] = getLux();
    doc["moisture"] = getMoisture();
    doc["created_at"] = time(NULL);

    //JSON구조체를 문자열로 변환
    char sendMessage[500];
    serializeJson(doc, sendMessage);
    if(!client.connected()){
      reconnect();
    }
    client.publish(pubTopic, sendMessage);
    Serial.println(F(sendMessage));
    
    
    delay(1000); //1초마다 전송
  }
}

//1번 코어
void receive(void *param){
  while(1){
    if(!client.connected()){
      continue;
    }
    else{
      // 클라이언트 수신 반복
      client.loop();
      delay(1000); //1초마다 받기
    }    
  }  
}

/*###############################################*/

//초기 설정
void setup() { 
  Serial.begin(9600);

  //WiFi연결
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.print(F("\nConnecting WiFi"));
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(F("."));
  }
  Serial.println(F("\nWiFi connected"));

  //Client연결
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callBack);    

  //Time셋팅
  configTime(gmtOffset_sec,daylightOffset_sec,ntpServer);
  Serial.println(F("Time loading..."));
  delay(5000);
  Serial.println(F("Get Local Time\n"));

  dht.begin();
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8); //LED
  ledcSetup(2, 5000, 8); //PUMP
  ledcAttachPin(MotorAA_LED, 0);
  ledcAttachPin(MotorAB_LED, 1);
  ledcAttachPin(MotorAA_PUMP, 0);
  ledcAttachPin(MotorAB_PUMP, 2);

  // 멀티스레드
  xTaskCreatePinnedToCore(
    send,     // Task를 구현할 함수명
    "Task1",  // Task를 이름
    10000,    // 스택 크기
    NULL,     // Task 파라미터
    1,        // Task 우선순위
    &Task1,   // Task 핸들
    0         // Task가 실행될 코어
  );

  xTaskCreatePinnedToCore(
    receive,
    "Task2",
    10000,
    NULL,
    1,
    &Task2,
    1
  );
}

void loop() {   
}
